#include "database_client.h"

namespace ebo
{
__Table::__Table(const __Table &tab)
    : db_(tab.db_), name_(tab.name_), field_map_(tab.field_map_)
{
    for (auto &[_, field] : field_map_)
    {
        field = field->Copy();
        fields_.push_back(field);
    }
    ClearCondition();
}


std::string __Table::GetRowStr() const
{
    std::string row = "(";
    bool first = true;
    for (auto &field : fields_)
    {
        if (!first) row += ", ";
        else first = false;
        row += field->GetName() + " = " + field->ToSqlStr();
    }
    row += ")";
    return row;
}

void __Table::ClearCondition()
{
    for (auto &field : fields_)
    {
        field->condition_ = 0;
    }
}

std::string __Field::StrToHex(const std::string &str)
{
    static const char *index = "0123456789ABCDEF";
    std::string buf;
    for (char cur : str)
    {
        buf += index[(cur >> 4)] + index[(cur % (1 << 4))];
    }
    return buf;
}

} // namespace ebo
