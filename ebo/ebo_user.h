#pragma once

#include "database_client.h"
#include "mysql_client.h"

namespace ebo
{
DECLARE_TABLE_CLASS(EboUser, __MysqlTable)
DECLARE_FIELD(__MysqlField, user_id, TYPE_INT, NOT_NULL | PRIMARY_KEY | AUTOINCREMENT, "", "", "")
DECLARE_FIELD(__MysqlField, user_qq, TYPE_VARCHAR, NOT_NULL, "(15)", "", "")  //QQ号
DECLARE_FIELD(__MysqlField, user_phone, TYPE_VARCHAR,  DEFAULT, "(11)", "18888888888", "")  //手机
DECLARE_FIELD(__MysqlField, user_name, TYPE_TEXT,  NOT_NULL, "", "", "")    //姓名
DECLARE_FIELD(__MysqlField, user_nick, TYPE_TEXT,  NOT_NULL, "", "", "")    //昵称
DECLARE_FIELD(__MysqlField, user_wechat, TYPE_TEXT,  DEFAULT, "", "NULL", "")
DECLARE_FIELD(__MysqlField, user_wechat_id, TYPE_TEXT,  DEFAULT, "", "NULL", "")
DECLARE_FIELD(__MysqlField, user_address, TYPE_TEXT,  DEFAULT, "", "", "")
DECLARE_FIELD(__MysqlField, user_province, TYPE_TEXT,  DEFAULT, "", "", "")
DECLARE_FIELD(__MysqlField, user_country, TYPE_TEXT, DEFAULT, "", "", "")
DECLARE_FIELD(__MysqlField, user_age, TYPE_INT, DEFAULT | CHECK, "", "18", "")
DECLARE_FIELD(__MysqlField, user_male, TYPE_INT, DEFAULT, "", "1", "")
DECLARE_FIELD(__MysqlField, user_flags, TYPE_TEXT, DEFAULT, "", "0", "")
DECLARE_FIELD(__MysqlField, user_experience, TYPE_REAL, DEFAULT, "", "0.0", "")
DECLARE_FIELD(__MysqlField, user_level, TYPE_INT, DEFAULT | CHECK, "", "0", "")
DECLARE_FIELD(__MysqlField, user_class_priority, TYPE_TEXT, DEFAULT, "", "", "")
DECLARE_FIELD(__MysqlField, user_time_per_viewer, TYPE_REAL, DEFAULT, "", "", "")
DECLARE_FIELD(__MysqlField, user_career, TYPE_TEXT, NONE, "", "", "")
DECLARE_FIELD(__MysqlField, user_password, TYPE_TEXT, NOT_NULL, "", "", "")
DECLARE_FIELD(__MysqlField, user_birthday, TYPE_INT, NONE, "", "", "")
DECLARE_FIELD(__MysqlField, user_describe, TYPE_TEXT, NONE, "", "", "")
DECLARE_FIELD(__MysqlField, user_education, TYPE_TEXT, NONE, "", "", "")
DECLARE_FIELD(__MysqlField, user_register_time, TYPE_DATETIME, DEFAULT, "", "NOW()", "")
DECLARE_TABLE_END()
    
} // namespace ebo
