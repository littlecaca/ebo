#include "sqlite3_client.h"

#include "logger.h"

using namespace ebo;

DECLARE_TABLE_CLASS(User, __Sqlite3Table);
DECLARE_FIELD(__Sqlite3Field, "user_id", TYPE_INT, 
    AUTOINCREMENT | PRIMARY_KEY | NOT_NULL, "(32)", "", "");
DECLARE_FIELD(__Sqlite3Field, "user_name", TYPE_VARCHAR,
    NOT_NULL | DEFAULT, "(100)", "unknown", "");
DECLARE_TABLE_END();


int main(int argc, char const *argv[])
{

    LOG_INFO << "sqlite3_client_test Start";
    DBPtr client = std::make_shared<Sqlite3Client>("test");
    KeyValue args {{"host", "test.db"}};
    client->Connect(args);

    User user(client);
    user.Create();
    Result result;
    user.Query(result);
    for (auto &tab : result)
    {
        LOG_INFO << tab->GetRowStr();
    }


    user["user_name"].condition_ = FieldOps::CONDITION;
    user["user_name"] = "mumu";
    User val1(user);
    val1.ClearCondition();
    val1["user_name"].condition_ = FieldOps::MODIFY;
    val1["user_name"] ="lala";
    user.Modify(val1);

    user["user_name"] = "kuku";
    user["user_name"].condition_ = FieldOps::INSERT;
    user.Insert();
    user.ClearCondition();

    result.clear();
    user.Query(result);
    for (auto &tab : result)
    {
        LOG_INFO << tab->GetRowStr();
    }

    client->Close();

    return 0;
}
