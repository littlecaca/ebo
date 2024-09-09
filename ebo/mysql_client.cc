#include "mysql_client.h"

#include <unordered_set>

#include "logger.h"

namespace ebo
{

namespace 
{
const char *kMysqlType[] = {
    "NULL",
    "BOOL",
    "INTEGER",
    "DATETIME",
    "REAL",
    "VARCHAR",
    "TEXT",
    "BLOB"
};

const std::unordered_set<std::string> types_no_default
{
    "BLOB",
    "TEXT",
    "GEOMETRY",
    "JSON"
};
}   // internal linkage

bool MysqlClient::Connect(const KeyValue &args)
{
    if (IsConnected())
    {
        LOG_ERROR << "MysqlClient::Connect() Connection has been established";
        return false;
    }
    auto host_it = args.find("host");
    auto port_it = args.find("port");
    auto user_it = args.find("user");
    auto pwd_it = args.find("pwd");
    auto db_it = args.find("db");

    if (host_it == args.end() || user_it == args.end()
        || pwd_it == args.end() || port_it == args.end() 
        || db_it == args.end())
    {
        LOG_ERROR << "MysqlClient::Connect() Can not find necessary args";
        return false;
        
    }
    MYSQL *ret = mysql_init(&db_);
    if (ret == nullptr)
    {
        LOG_ERROR << "MysqlClient::Connect() Init fails: " << mysql_errno(&db_)
            << " " << mysql_error(&db_);
        return false;
    }

    ret = mysql_real_connect(&db_, host_it->second.data(), user_it->second.data(),
        pwd_it->second.data(), db_it->second.data(), std::stoul(port_it->second), nullptr, 0);
    
    if (ret == nullptr || mysql_errno(&db_) != 0)
    {
        LOG_ERROR << "MysqlClient::Connect() Connect fails: " << mysql_errno(&db_)
            << " " << mysql_error(&db_);
        mysql_close(&db_);
        ::memset(&db_, 0, sizeof db_);
        return false;
    }

    connected_ = true;
    return true;
}

bool MysqlClient::Exec(const std::string &sql)
{
    return ExecCommand(sql.data(), sql.size());
}

bool MysqlClient::Exec(const std::string &sql, Result &result, __Table &table)
{
    if (!ExecCommand(sql.data(), sql.size()))
        return false;
    MYSQL_RES *res = mysql_store_result(&db_);

    MYSQL_ROW row;
    unsigned num_fields = mysql_num_fields(res);

    while ((row = mysql_fetch_row(res)) != nullptr)
    {
        TablePtr tab = table.Copy();
        for (unsigned i = 0; i < num_fields; ++i)
        {
            if (row[i] != nullptr)
            {
                tab->FieldDefine()[i]->LoadFromStr(row[i]);
                // DEBUGINFO << tab->FieldDefine()[i]->GetName() << ": " << row[i];
            }
        }
        result.push_back(tab);
    }
    return true;
}

bool MysqlClient::Close()
{
    if (IsConnected())
    {
        mysql_close(&db_);
        ::memset(&db_, 0, sizeof db_);
        connected_ = false;
    }
    return true;
}

bool MysqlClient::ExecCommand(const char *command, size_t len)
{
    if (!IsConnected())
    {
        LOG_ERROR << "MysqlClient::ExecCommand() Connection has not been established";
        return false;
    }

    int ret = mysql_real_query(&db_, command, len);
    
    if (ret != 0)
    {
        LOG_ERROR << "MysqlClient::ExecCommand() " << command 
            << " execution fails: " << mysql_errno(&db_) << " " << mysql_error(&db_);
        return false;
    }
    return true;
}

bool __MysqlTable::Create()
{
    // CREATE TABLE IF NOT EXISTS `tablename` `field`, `field`...;
    std::string sql = "CREATE TABLE IF NOT EXISTS " + GetName() + " (\r\n";
    bool first = true;
    std::string primary = "";
    std::string unique = "";

    for (auto &field : fields_)
    {
        if (!first) sql += ",\r\n";
        else first = false;
        if (field->GetAttr() & PRIMARY_KEY)
        {
            if (primary.size())
                primary += ", ";
            primary += field->ToSqlName();
        }
        if (field->GetAttr() & UNIQUE)
        {
            if (unique.size())
                unique += ", ";
            unique += field->ToSqlName();
        }
        
        sql += field->Create();
    }
    if (primary.size())
    {
        sql += ",\r\n";
        sql += "PRIMARY KEY(" + primary + ")";
    }
    if (unique.size())
    {
        sql += ",\r\n";
        sql += "UNIQUE(" + unique + ")";
    }

    sql += ");";
    DEBUGINFO << "__MysqlTable::Create() " << sql;
    return db_->Exec(sql);
}

bool __MysqlTable::Drop()
{
    std::string sql = "DROP TABLE " + GetName() + ";";
    DEBUGINFO << "__MysqlTable::Drop() " << sql;

    return db_->Exec(sql);
}

bool __MysqlTable::Insert()
{
    std::string sql = "INSERT INTO " + GetName() + " (";
    bool first = true;
    for (auto &field : fields_)
    {
        if (field->condition_ & FieldOps::INSERT)
        {
            if (!first) sql += ", ";
            else first = false;
            sql += field->ToSqlName();
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

    DEBUGINFO << "__MysqlTable::Insert() " << sql;
    
    return db_->Exec(sql);
}

bool __MysqlTable::Delete()
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
    DEBUGINFO << "__MysqlTable::Delete() " << sql;

    return db_->Exec(sql);
}

bool __MysqlTable::Modify(const __Table &val)
{
    std::string sql = "UPDATE " + GetName() + " SET ";
    bool first = true;
    for (auto &field : val.FieldDefine())
    {
        if (field->condition_ & FieldOps::MODIFY)
        {
            if (!first) sql += ", ";
            else first = false;
            sql += field->ToSqlName() + " = " + field->ToSqlStr();
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
    DEBUGINFO << "__MysqlTable::Modify() " << sql;
    
    return db_->Exec(sql);
}

bool __MysqlTable::Query(Result &result, const std::string &condition)
{
    std::string sql = "SELECT ";
    bool first = true;
    for (auto &field : fields_)
    {
        if (!first) sql += ", ";
        else first = false;
        sql += field->ToSqlName();
    }
    sql += " FROM " + GetName();

    if (condition.size())
    {
        sql += " WHERE " + condition;
    }
    sql += ";";

    DEBUGINFO << "__MysqlTable::Query() " << sql;
    return db_->Exec(sql, result, *this);
}

TablePtr __MysqlTable::Copy() const
{
    return std::make_shared<__MysqlTable>(*this);
}

__MysqlField::__MysqlField(
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
        n_type_(type) 
{
    Double = 0;
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
__MysqlField::~__MysqlField()
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
__MysqlField::__MysqlField(const __MysqlField &field)
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

std::string __MysqlField::Create()
{
    std::string sql = ToSqlName() + " " + type_;
    if (size_.size()) sql += size_;

    if (attr_ & NOT_NULL)
    {
        sql += " NOT NULL";
    }
    else 
    {
        sql += " NULL";
    }

    if ((attr_ & DEFAULT) && default_.size() && !types_no_default.count(type_))
    {
        if (n_type_ != TYPE_DATETIME)
            sql += " DEFAULT \"" + default_ + "\"";
        else
            sql += " DEFAULT " + default_;
    }
    if (attr_ & AUTOINCREMENT)
    {
        sql += " AUTO_INCREMENT";
    }
    return sql;
}

void __MysqlField::LoadFromStr(const std::string &str)
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
        try
        {
            Integer = std::stoll(str.data());
        }
        catch(const std::out_of_range& e)
        {
            LOG_ERROR << "__MysqlField::LoadFromStr() " << GetName() << " type: " 
                        << type_ << ": " << e.what();
            Integer = -1;
        }
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

std::string __MysqlField::ToEqualExp()
{
    return ToSqlName() + " " + com_ops_ + " " + ToSqlStr();
}

std::string __MysqlField::ToSqlStr()
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

FieldPtr __MysqlField::Copy() const
{
    return std::make_shared<__MysqlField>(*this);
}

const char *__MysqlField::SqlTypeToStr(unsigned type)
{
    return kMysqlType[type];
}

} // namespace ebo
