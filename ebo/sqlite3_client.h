#pragma once

#include <memory>

#include "database_client.h"
#include "sqlite3.h"
#include "noncopyable.h"

namespace ebo
{
class Sqlite3Client : public DatebaseClient
{
public:
    Sqlite3Client(const std::string &name = "") 
        : DatebaseClient(name), stmt_(nullptr), db_(nullptr)
    {
    }
    
    virtual ~Sqlite3Client()
    {
    }

public:
    virtual bool Connect(const KeyValue &args) override;
    virtual bool Exec(const std::string &sql) override;
    virtual bool Exec(const std::string &sql, Result &result, __Table &table) override;

    virtual bool StartTransaction() override
    {
        return ExecCommand("BEGIN TRANSACTION");
    }
    virtual bool CommitTransaction() override
    {
        return ExecCommand("COMMIT TRANSACTION");
    }
    virtual bool RollbackTransaction() override
    {
        return ExecCommand("ROLLBACK TRANSACTION");
    }

    virtual bool Close() override;
    virtual bool IsConnected() const override
    {
        return db_ != nullptr;
    }

private:
    static int ExecCallback(void* arg, int count, char** names, char** values);
    int ExecCallback(Result &result, __Table& table, int count, char** names, char** values);

    bool ExecCommand(const char *command) const;

private:
    sqlite3_stmt *stmt_;
    sqlite3 *db_;
};


class __Sqlite3Table : public __Table
{
public:
    __Sqlite3Table(DBPtr db, const std::string &name)
        : __Table(db, name)
    {}

    __Sqlite3Table(const __Sqlite3Table &tab)
        : __Table(tab)
    {}

public:
    virtual bool Create() override;
    virtual bool Drop() override;
    virtual bool Insert() override;
    virtual bool Delete() override;
    virtual bool Modify(const __Table &val) override;
    virtual bool Query(Result &result) override;

    virtual TablePtr Copy() const override;
};

class __Sqlite3Field : public __Field
{
public:
    __Sqlite3Field(
        const std::string &name,
        unsigned type,
        unsigned attr,
        const std::string &size,
        const std::string &_default,
        const std::string &check
    );

    ~__Sqlite3Field() override;

    __Sqlite3Field(const __Sqlite3Field &field);

public:
    virtual std::string Create() override;
    virtual void LoadFromStr(const std::string &str) override;
    virtual std::string ToEqualExp() override;
    virtual std::string ToSqlStr() override;
    virtual FieldPtr Copy() const override;
    virtual __Field &operator=(const std::string &str) override
    {
        LoadFromStr(str);
        return *this;
    }

protected:
    static std::string StrToHex(const std::string &str);

    static const char *SqlTypeToStr(unsigned type);

protected:
    union
    {
        bool Bool;
        int Integer;
        double Double;
        std::string *String;
    };
    unsigned n_type_;
};




} // namespace ebo
