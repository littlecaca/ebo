#include "sqlite3_client.h"

#include "logger.h"

namespace ebo
{

namespace
{
struct ExecParam
{
    Sqlite3Client *obj_;
    Result &result_;
    __Table &table_;
};

const char *kSqlite3Type[] = {
    "NULL",
    "BOOL",
    "INTEGER",
    "DATETIME",
    "REAL",
    "VARCHAR",
    "TEXT",
    "BLOB"
};
}   // internal linkage

bool Sqlite3Client::Connect(const KeyValue &args)
{
    if (IsConnected())
    {
        LOG_ERROR << "Sqlite3Client::Connect() Connection has been established";
        return false;
    }
    auto host_it = args.find("host");
    if (host_it == args.end())
    {
        LOG_ERROR << "Sqlite3Client::Connect() Can not find host";
        return false;
    }

    int ret = sqlite3_open(host_it->second.data(), &db_);
    if (ret != 0)
    {
        LOG_ERROR << "Sqlite3Client::Connect() Connect fails";
        return false;
    }
    return true;
}

bool Sqlite3Client::Exec(const std::string &sql)
{
    if (!IsConnected())
    {
        LOG_ERROR << "Sqlite3Client::Exec() Connection has not been established";
        return false;
    }
    int ret = sqlite3_exec(db_, sql.data(), nullptr, this, nullptr);
    if (ret != SQLITE_OK)
    {
        LOG_ERROR << "Sqlite3Client::Exec() execution fails: " << sql 
                  << " : " << sqlite3_errmsg(db_);
        return false;
    }

    return true;
}

bool Sqlite3Client::Exec(const std::string &sql, Result &result, __Table &table)
{
    char *errmsg = nullptr;
    if (!IsConnected())
    {
        LOG_ERROR << "Sqlite3Client::Exec() Connection has not been established";
        return false;
    }
    ExecParam param{this, result, table};
    int ret = sqlite3_exec(db_, sql.data(), &Sqlite3Client::ExecCallback,
        static_cast<void *>(&param), &errmsg);
    if (ret != SQLITE_OK)
    {
        LOG_ERROR << "Sqlite3Client::Exec() execution fails: " 
                  << sql << " : " << errmsg;
        return false;
    }
    if (errmsg) ::free(errmsg);

    return true;
}

int Sqlite3Client::ExecCallback(void *arg, int count, char **values, char **names)
{
    ExecParam *param = static_cast<ExecParam *>(arg);
    return param->obj_->ExecCallback(param->result_, param->table_, count, names, values);
}

int Sqlite3Client::ExecCallback(Result &result, __Table &table, int count, 
    char **names, char **values)
{
    TablePtr tab = table.Copy();

    for (int i = 0; i < count; ++i)
    {
        auto field_it = tab->Fields().find(names[i]);
        if (field_it == tab->Fields().end())
        {
            LOG_ERROR << "Sqlite3Client::ExecCallback() Can not find name: " << names[i];
            return -1;
        }
        if (values[i] != nullptr)
        {
            field_it->second->LoadFromStr(values[i]);
        }
    }
    result.push_back(tab);

    return 0;
}

bool Sqlite3Client::ExecCommand(const char *command) const
{
    if (!IsConnected())
    {
        LOG_ERROR << "Sqlite3Client::ExecCommand() Connection has not been established";
        return false;
    }

    int ret = sqlite3_exec(db_, "ROLLBACK TRANSACTION", nullptr, nullptr, nullptr);
    if (ret != SQLITE_OK)
    {
        LOG_ERROR << "Sqlite3Client::ExecCommand() " << command 
                  << " execution fails: " << sqlite3_errmsg(db_);
        return false;
    }
    return true;
}

bool Sqlite3Client::Close()
{
    if (!IsConnected())
    {
        LOG_ERROR << "Sqlite3Client::Close() Connection has not been established";
        return false;
    }

    int ret = sqlite3_close(db_);
    if (ret != SQLITE_OK)
    {
        LOG_ERROR << "Sqlite3Client::Close() fails" << sqlite3_errmsg(db_);
        return false;
    }
    db_ = nullptr;
    return true;
}

bool __Sqlite3Table::Create()
{
    // CREATE TABLE IF NOT EXISTS `tablename` `field`, `field`...;
    std::string sql = "CREATE TABLE IF NOT EXISTS " + GetName() + " (\r\n";
    bool first = true;
    for (auto &field : fields_)
    {
        if (!first) sql += ", ";
        else first = false;
        sql += field->Create(); 
    }

    sql += ");";
    DEBUGINFO << "__Sqlite3Table::Create() " << sql;

    return db_->Exec(sql);
}

bool __Sqlite3Table::Drop()
{
    std::string sql = "DROP TABLE " + GetName() + ";";
    DEBUGINFO << "__Sqlite3Table::Drop() " << sql;

    return db_->Exec(sql);
}

bool __Sqlite3Table::Insert()
{
    std::string sql = "INSERT INTO " + GetName() + " (";
    bool first = true;
    for (auto &field : fields_)
    {
        if (field->condition_ & FieldOps::INSERT)
        {
            if (!first) sql += ", ";
            else first = false;
            sql += field->GetName();
        }
    }
    sql += ") VALUES (";

    first = true;

    for (auto &field : fields_)
    {
        if (field->condition_ & FieldOps::INSERT)
        {
            if (!first) sql += ", ";
            else first = false;
            sql += field->ToSqlStr();
        }
    }
    sql += ");";

    DEBUGINFO << "__Sqlite3Table::Insert() " << sql;
    
    return db_->Exec(sql);
}

bool __Sqlite3Table::Delete()
{
    std::string sql = "DELETE FROM " + GetName();
    
    std::string Where =  "";
    bool first = true;

    for (auto &field : fields_)
    {
        if (field->condition_ & FieldOps::CONDITION)
        {
            if (!first) sql += " AND ";
            else first = false;
            sql += field->ToEqualExp();
        }
    }

    if (Where.size())
    {
        sql += " WHRER " + Where;
    }
    sql += ";";
    DEBUGINFO << "__Sqlite3Table::Delete() " << sql;

    return db_->Exec(sql);
}

bool __Sqlite3Table::Modify(const __Table &val)
{
    std::string sql = "UPDATE " + GetName() + " SET ";
    bool first = true;
    for (auto &field : val.FieldDefine())
    {
        if (field->condition_ & FieldOps::MODIFY)
        {
            if (!first) sql += ", ";
            else first = false;
            sql += field->GetName() + " = " + field->ToSqlStr();
        }
    }

    std::string Where = "";
    first = true;

    for (auto &field : fields_)
    {
        if (field->condition_ & FieldOps::CONDITION)
        {
            if (!first) Where += " AND ";
            else first = false;
            Where += field->ToEqualExp();
        }
    }

    if (Where.size())
    {
        sql += " WHERE " + Where;
    }
    sql += ";";
    DEBUGINFO << "__Sqlite3Table::Modify() " << sql;

    
    return db_->Exec(sql);
}

bool __Sqlite3Table::Query(Result &result)
{
    std::string sql = "SELECT ";
    bool first = true;
    for (auto &field : fields_)
    {
        if (!first) sql += ", ";
        else first = false;
        sql += "\"" + field->GetName() + "\"";
    }
    sql += " FROM " + GetName() + ";";

    DEBUGINFO << "__Sqlite3Table::Query() " << sql;
    return db_->Exec(sql, result, *this);
}

TablePtr __Sqlite3Table::Copy() const
{
    return std::make_shared<__Sqlite3Table>(*this);
}

__Sqlite3Field::__Sqlite3Field(
    const std::string &name, unsigned type, unsigned attr, 
    const std::string &size, const std::string &_default, 
    const std::string &check)
        : 
        __Field(
            name,
            SqlTypeToStr(type),
            attr,
            size,
            _default,
            check
        ),
        Double(0),
        n_type_(type) 
{
    switch (n_type_)
    {
    case TYPE_VARCHAR:
    case TYPE_TEXT:
    case TYPE_BLOB:
        String = new std::string;
        break;
    default:
        break;
    }
}

__Sqlite3Field::~__Sqlite3Field()
{
    switch (n_type_)
    {
    case TYPE_VARCHAR:
    case TYPE_TEXT:
    case TYPE_BLOB:
        delete String;
        String = nullptr;
        break;
    default:
        break;
    }
}

__Sqlite3Field::__Sqlite3Field(const __Sqlite3Field &field)
    : __Field(field), n_type_(field.n_type_)
{
    switch (field.n_type_)
    {
    case TYPE_BOOL:
    case TYPE_INT:
    case TYPE_DATETIME:
        Integer = field.Integer;
        break;
    case TYPE_REAL:
        Double = field.Double;
        break;
    case TYPE_VARCHAR:
    case TYPE_TEXT:
    case TYPE_BLOB:
        String = new std::string(*field.String);
        break;
    default:
        break;
    }
}

std::string __Sqlite3Field::Create()
{
    std::string sql = "\"" + name_ + "\" " + type_;
    if (size_.size()) sql += size_;

    if (attr_ & NOT_NULL)
    {
        sql += " NOT NULL";
    }
    if (attr_ & DEFAULT)
    {
        sql += " DEFAULT " + default_;
    }
    if (attr_ & UNIQUE)
    {
        sql += " UNIQUE";
    }
    if (attr_ & PRIMARY_KEY)
    {
        sql += " PRIMARY KEY";
    }
    if (attr_ & CHECK)
    {
        sql += " CHECK( " + check_ +  " ) ";
    }
    if (attr_ & AUTOINCREMENT)
    {
        sql += " AUTOINCREMENT";
    }
    return sql;
}

void __Sqlite3Field::LoadFromStr(const std::string &str)
{
    switch (n_type_)
    {
    case TYPE_VARCHAR:
    case TYPE_TEXT:
    case TYPE_BLOB:
        delete String;
        String = nullptr;
        break;
    default:
        break;
    }

    switch (n_type_)
    {
    case TYPE_NULL:
        break;
    case TYPE_BOOL:
    case TYPE_INT:
    case TYPE_DATETIME:
        Integer = std::stoi(str);
        break;
    case TYPE_REAL:
        Double = std::stod(str);
        break;
    case TYPE_TEXT:
    case TYPE_VARCHAR:
        String = new std::string(str);
        break;
    case TYPE_BLOB:
        String = new std::string(StrToHex(str));
        break;
    default:
        break;
    }
}

std::string __Sqlite3Field::ToEqualExp()
{
    return name_ + " " + com_ops_ + " " + ToSqlStr();
}

std::string __Sqlite3Field::ToSqlStr()
{
    switch (n_type_)
    {
    case TYPE_NULL:
        return "NULL";
    case TYPE_BOOL:
    case TYPE_INT:
    case TYPE_DATETIME:
        return std::to_string(Integer);
        break;
    case TYPE_REAL:
        return std::to_string(Double);
        break;
    case TYPE_TEXT:
    case TYPE_VARCHAR:
    case TYPE_BLOB:
        return "\"" + *String + "\"";
    default:
        break;
    }
    LOG_ERROR << "Invalid n_type_ " << static_cast<unsigned>(n_type_);
    return "???";
}

FieldPtr __Sqlite3Field::Copy() const
{
    return std::make_shared<__Sqlite3Field>(*this);
}

std::string __Sqlite3Field::StrToHex(const std::string &str)
{
    static const char *index = "0123456789ABCDEF";
    std::string buf;
    for (char cur : str)
    {
        buf += index[(cur >> 4)] + index[(cur % (1 << 4))];
    }
    return buf;
}

const char *__Sqlite3Field::SqlTypeToStr(unsigned type)
{
    return kSqlite3Type[type];
}

} // namespace ebo
