#pragma once


#include <vector>
#include <string>
#include <map>
#include <list>
#include <memory>

#include "noncopyable.h"   

namespace ebo
{
using KeyValue = std::map<std::string, std::string>;

class __Table;
class __Field;
using TablePtr = std::shared_ptr<__Table>;
using FieldPtr = std::shared_ptr<__Field>;
using FieldList = std::vector<FieldPtr>;
using FieldMap = std::map<std::string, FieldPtr>;
using Result  = std::vector<TablePtr>;
using RowList = std::vector<FieldMap>;

class DatebaseClient : 
    public std::enable_shared_from_this<DatebaseClient>, 
    muzi::noncopyable
{
public:
    DatebaseClient(const std::string &name) 
        : name_(name) 
    {}
    virtual ~DatebaseClient()
    {
    }

public:
    virtual bool Connect(const KeyValue &args) = 0;
    virtual bool Exec(const std::string &sql) = 0;
    virtual bool Exec(const std::string &sql, Result &result, __Table &table) = 0;

    virtual bool StartTransaction() = 0;
    virtual bool CommitTransaction() = 0;
    virtual bool RollbackTransaction() = 0;
    virtual bool Close() = 0;
    virtual bool IsConnected() const = 0;

public:
    const std::string &GetName() const { return name_; }

private:
    std::string name_;
};

using DBPtr = std::shared_ptr<DatebaseClient>;

class __Table
{
public:
    __Table(DBPtr db, const std::string &name)
        : db_(db), name_(name)
    {
    }

    __Table(const __Table &tab);
       
    virtual ~__Table() = default;

public:
    virtual bool Create() = 0;
    virtual bool Drop() = 0;
    virtual bool Insert() = 0;
    virtual bool Delete() = 0;
    virtual bool Modify(const __Table &val) = 0;
    virtual bool Query(Result &result) = 0;

    virtual TablePtr Copy() const = 0;
    
    const std::string &GetName() const { return name_; }
    const std::string &GetDBName() const { return db_->GetName(); } 
    FieldMap &Fields() { return field_map_; }
    const FieldMap &Fields() const { return field_map_; }
    FieldList &FieldDefine() { return fields_; }
    const FieldList &FieldDefine() const { return fields_; }
    __Field &operator[](const std::string &key) { return *field_map_.at(key);}
    std::string GetRowStr() const;
    void ClearCondition();

protected:
    DBPtr db_;
    std::string name_;

    FieldList fields_;
    FieldMap field_map_;
};

enum FieldOps : unsigned
{
    INSERT = 1,
    MODIFY = 2,
    CONDITION = 4,
};

enum FieldAttr : unsigned
{
    NOT_NULL = 1,
    DEFAULT = 2,
    UNIQUE = 4,
    PRIMARY_KEY = 8,
    CHECK = 16,
    AUTOINCREMENT = 32
};

enum SqlType : unsigned
{
    TYPE_NULL,
    TYPE_BOOL,
    TYPE_INT,
    TYPE_DATETIME, 
    TYPE_REAL,
    TYPE_VARCHAR,
    TYPE_TEXT,
    TYPE_BLOB,
};

class __Field
{
public:
    __Field(
        const std::string &name,
        const std::string &type,
        unsigned attr,
        const std::string &size,
        const std::string &_default,
        const std::string &check
    )
        : name_(name), type_(type), attr_(attr),
          size_(size), default_(_default), check_(check),
          com_ops_("=")
    {
    }
    virtual ~__Field() = default;

public:
    virtual std::string Create() = 0;
    virtual void LoadFromStr(const std::string &str) = 0;
    virtual __Field &operator=(const std::string &str) = 0;
    virtual std::string ToEqualExp() = 0;
    virtual std::string ToSqlStr() = 0;
    virtual FieldPtr Copy() const = 0;

    const std::string &GetName() const { return name_; }
    void SetComOps(const std::string &ops) { com_ops_ = ops; }

public:
    unsigned condition_;

protected:
    std::string name_;
    std::string type_;
    unsigned attr_;
    std::string size_;;
    std::string default_;
    std::string check_;

    std::string com_ops_;
};

#define DECLARE_TABLE_CLASS(name, base)                                 \
    class name : public base                                            \
    {                                                                   \
    public :                                                            \
        virtual TablePtr Copy() { return TablePtr(new name(*this)); }       \
        name(DBPtr db) : base(db, #name) {                              

#define DECLARE_FIELD(field_name, name, n_type, attr, size, _default, check)        \
    {FieldPtr field(new field_name(name, n_type, attr, size, _default, check));     \
        fields_.push_back(field); field_map_[name] = field;}

#define DECLARE_TABLE_END() }};


}   // namespace ebo
