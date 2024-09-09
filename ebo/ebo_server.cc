#include "ebo_server.h"

#include <fstream>
#include <unordered_map>
#include <functional>

#include "logger.h"
#include "mysql_client.h"
#include "ebo_user.h"
#include "http_parser.h"
#include "encryptor.h"

using namespace std::placeholders;

namespace ebo
{
namespace
{
const char *ToStatusMessage(int status_code)
{
    static std::unordered_map<int, const char *> status_messages = 
    {
        {200, "OK"},
        {201, "Created"},
        {202, "Accepted"},
        {204, "No Content"},
        {301, "Moved Permanently"},
        {302, "Found"},
        {304, "Not Modified"},
        {400, "Bad Request"},
        {401, "Unauthorized"},
        {403, "Forbidden"},
        {404, "Not Found"},
        {500, "Internal Server Error"},
        {502, "Bad Gateway"},
        {503, "Service Unavailable"}
    };

    auto it = status_messages.find(status_code);
    if (it != status_messages.end())
    {
        return it->second;
    }
    return "";
}

}   // internal linkage


void EboServer::Init()
{
    // Set server callback
    server_.SetMessageCallback(std::bind(&EboServer::OnMessage, this, _1, _2, _3));
    server_.SetConnectionCallback(std::bind(&EboServer::OnConnection, this, _1));

    // Read the mysql login info
    std::ifstream config("config.json");
    if (!config.is_open())
    {
        LOG_FATAL << "Can not open the config.json";
    }

    Json::Reader reader;
    Json::Value root;
    writer_["emitUTF8"] = true;

    if (!reader.parse(config, root))
    {
        LOG_FATAL << "config.json is not a valid json file";
    }
    // Retrive md5_key
    md5_key_ = root["safe"]["md5_key"].asString();

    // Login db user
    const Json::Value mysql_config = root["mysql"];

    db_args_["host"] = mysql_config["host"].asString();
    db_args_["port"] = mysql_config["port"].asString();
    db_args_["user"] = mysql_config["user"].asString();
    db_args_["pwd"] = mysql_config["pwd"].asString();
    db_args_["db"] = mysql_config["db"].asString();

    // Connect to the mysql server and create the user table if not exists
    db_ = std::make_shared<MysqlClient>("ebo");
    db_->Connect(db_args_);

    user_tab_ = std::make_shared<EboUser>(db_);
    user_tab_->Create();
    db_->Close();
}

void EboServer::OnMessage(const muzi::TcpConnectionPtr &conn, 
    muzi::Buffer *buf, muzi::Timestamp time)
{
    // DEBUGINFO << "EboServer::OnMessage() Get message: " << buf->PeekAllAsString();
    std::shared_ptr<HttpMessage> msg_ptr;
    std::any context = conn->GetContext();
    if (context.has_value())
    {
        msg_ptr = std::any_cast<std::shared_ptr<HttpMessage>>(context);
    }
    else
    {
        msg_ptr = std::make_shared<HttpMessage>();
        conn->SetContext(msg_ptr);
    }
    
    // DEBUGINFO << "buf->ReadableBytes(): " << buf->ReadableBytes();
    while (buf->ReadableBytes())
    {
        size_t len = msg_ptr->Parse(buf->PeekAllAsString());
        buf->Retrive(len);
        // DEBUGINFO << "Readable bytes";
        if (msg_ptr->IsMessageParsed())
        {
            // DEBUGINFO << "MessageParsed";
            Dispath(conn, *msg_ptr, time);
        }
    }
}

void EboServer::Dispath(const muzi::TcpConnectionPtr &conn, 
    const HttpMessage &msg, muzi::Timestamp time)
{
    // DEBUGINFO << "EboServer::Dispath() ";
    Url url("http://127.0.0.1" + msg.Url());
    // DEBUGINFO << "msg.Url() " << msg.Url();
    // DEBUGINFO << "urlpath " << url.UrlPath();
    if (url.IsValid())
    {
        if (msg.Method() == HTTP_GET 
            && (url.UrlPath() == "login" || url.UrlPath() == "/login"))
        {
            OnLogin(conn, msg);
            return;
        }
    }
    
    // Wrong url
    LOG_INFO << "Wrong url from " << conn->GetName() << ": " << url.UrlPath();

    static std::string not_found = "<h1>404 Not Found</h1>\r\n";
    Response(conn, HTTP_STATUS_NOT_FOUND, not_found);
}

void EboServer::OnLogin(const muzi::TcpConnectionPtr &conn, const HttpMessage &msg)
{
    // DEBUGINFO << "entry OnLogin()";

    Json::Value ret;
    ret["message"] = "登录失败，请检查用户名或密码";
    
    Url url("http://127.0.0.1" + msg.Url());
    
    try
    {
        std::string time = url.At("time");
        std::string salt = url.At("salt");
        std::string user = url.At("user");
        std::string sign = url.At("sign");

        DEBUGINFO << "time: " << time << " salt: " << salt 
                  << " user: " << user << " sign: " << sign;
        
        // Query the db
        db_->Connect(db_args_);
        Result result;
        user_tab_->Query(result, "`user_name`=\"" + user + "\"");
        
        if (result.size() != 1)
        {
            ret["status"] = "-2";
        }
        else
        {
            // Verify
            TablePtr res = result[0];
          
            std::string password = *(*res)["user_password"].String;

            std::string md5 = time + md5_key_ + password + salt;
            // DEBUGINFO << md5;
            std::string expected_sign = Encryptor::MD5(md5);
            // DEBUGINFO << expected_sign;
            if (expected_sign != sign)
            {
                ret["status"] = "-3";
            }
            else
            {
                ret["status"] = "0";
                ret["message"] = "Success";
            }
        }
    }
    catch (std::out_of_range &e)
    {
        DEBUGINFO << "Lack of necessary url args";
        LOG_ERROR << e.what();
        ret["status"] = "-1";
    }

    Response(conn, HTTP_STATUS_OK, ret);
}

void EboServer::Response(const muzi::TcpConnectionPtr &conn, 
    int status_code, const std::string &type, const std::string &body)
{
    std::string status = "HTTP/1.1 " + std::to_string(status_code) 
        + " " + ToStatusMessage(status_code) +  "\r\n";

    
    std::string headers = "Server: Ebo/1.0\r\n";
    headers += "Date: " + std::string(muzi::Timestamp().ToFormatString().data()) + "\r\n";
    headers += "Content-Type: " + type + "; charset=utf-8\r\n";
    headers += "X-Frame-Options: DENY\r\n";
    headers += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    headers += "X-Content-Type-Options: nosniff\r\n";
    headers += "Referer-Policy: same-origin\r\n";
    headers += "\r\n";

    conn->Send(status + headers + body);
}

void EboServer::OnConnection(const muzi::TcpConnectionPtr &conn)
{
    if (conn->IsConnected())
    {
        LOG_INFO << "EboServer::OnConnection() accpet new connection " << conn->GetName()
                 << " from " << conn->GetPeerAddress()->GetAddrStr();
    }
    else
    {
        LOG_INFO << "EboServer::OnConnection() close connection " << conn->GetName();
    }
    
}

} // namespace ebo
