#include "mysql_client.h"

#include "logger.h"

using namespace ebo;

DECLARE_TABLE_CLASS(User, __MysqlTable);
DECLARE_FIELD(__MysqlField, "user_id", TYPE_INT, 
    AUTOINCREMENT | PRIMARY_KEY | NOT_NULL, "", "", "");
DECLARE_FIELD(__MysqlField, "user_name", TYPE_VARCHAR,
    NOT_NULL | DEFAULT, "(100)", "unknown", "");
DECLARE_TABLE_END();


int main(int argc, char const *argv[])
{

    LOG_INFO << "sqlite3_client_test Start";
    DBPtr client = std::make_shared<MysqlClient>("test");
    KeyValue args;
    args["host"] = "192.168.154.129";
    args["port"] = "3306";
    args["user"] = "siso";
    args["pwd"] = "5632470";
    args["db"] = "ebo";
    
    client->Connect(args);

    User user(client);
    user.Create();
    Result result;
    user.Query(result);
    for (auto &tab : result)
    {
        LOG_INFO << tab->GetRowStr();
    }

    user["user_name"].condition_ = CONDITION;
    user["user_name"] = "mumu";
    User val1(user);
    val1["user_name"].condition_ = MODIFY;
    val1["user_name"] = "lala";
    user.Modify(val1);

    user["user_name"] = "kuku";
    user["user_name"].condition_ = INSERT;
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
