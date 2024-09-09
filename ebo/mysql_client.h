#pragma once

#include <memory.h>

#include <mysql/mysql.h>

#include "database_client.h"

namespace ebo
{
class MysqlClient : public DatebaseClient
{
public:
    MysqlClient(const std::string &name = "") 
        : DatebaseClient(name), connected_(false)
    {
        ::memset(&db_, 0, sizeof db_);
    }
    
    virtual ~MysqlClient()
    {
    }

public:
    virtual bool Connect(const KeyValue &args) override;
    virtual bool Exec(const std::string &sql) override;
    virtual bool Exec(const std::string &sql, Result &result, __Table &table) override;

    virtual bool StartTransaction() override
    {
        return ExecCommand("BEGIN", sizeof "BEGIN");
    }
    virtual bool CommitTransaction() override
    {
        return ExecCommand("COMMIT", sizeof "COMMIT");
    }
    virtual bool RollbackTransaction() override
    {
        return ExecCommand("ROLLBACK", sizeof "ROLLBACK");
    }

    virtual bool Close() override;
    virtual bool IsConnected() const override
    {
        return  connected_;
    }

private:
    bool ExecCommand(const char *command, size_t len);

private:
    bool connected_;
    MYSQL db_;
};

class __MysqlTable : public __Table
{
public:
    __MysqlTable(DBPtr db, const std::string &name)
        : __Table(db, name)
    {}

    __MysqlTable(const __MysqlTable &tab)
        : __Table(tab)
    {}

public:
    virtual bool Create() override;
    virtual bool Drop() override;
    virtual bool Insert() override;
    virtual bool Delete() override;
    virtual bool Modify(const __Table &val) override;
    virtual bool Query(Result &result, const std::string &condition = "") override;

    virtual TablePtr Copy() const override;
};


class __MysqlField : public __Field
{
public:
    __MysqlField(
        const std::string &name,
        unsigned type,
        unsigned attr,
        const std::string &size,
        const std::string &_default,
        const std::string &check
    );

    ~__MysqlField() override;

    __MysqlField(const __MysqlField &field);

public:
    virtual std::string Create() override;
    virtual void LoadFromStr(const std::string &str) override;
    virtual std::string ToEqualExp() override;
    virtual std::string ToSqlStr() override;
    virtual FieldPtr Copy() const override;
    virtual std::string ToSqlName() const override { return "`" + name_ + "`"; }

    virtual __Field &operator=(const std::string &str) override
    {
        LoadFromStr(str);
        return *this;
    }

protected:

    static const char *SqlTypeToStr(unsigned type);

protected:

    unsigned n_type_;
};
}   // namespace ebo
