#pragma once

#include <string>

#include "address.h"
#include "event_loop.h"
#include "tcp_server.h"
#include "http_parser.h"
#include "database_client.h"

#include <jsoncpp/json/json.h>

namespace ebo
{
class EboServer
{
public:
    EboServer(muzi::EventLoop *loop, const muzi::Address &listen_addr,
         const std::string &name)
        : server_(loop, listen_addr, name)
    {
        server_.SetThreadNum(3);
        Init();
    }

    void Start()
    {
        server_.Start();
    }

private:
    // Initialize the db
    void Init();

    // Dispath the requeset to handler
    void OnMessage(const muzi::TcpConnectionPtr &conn, muzi::Buffer *buf, muzi::Timestamp time);

    void Dispath(const muzi::TcpConnectionPtr &conn, const HttpMessage &msg, muzi::Timestamp time);

    void OnLogin(const muzi::TcpConnectionPtr &conn, const HttpMessage &msg);

    void Response(const muzi::TcpConnectionPtr &conn, int status_code,
        const Json::Value &body)
    {
        Response(conn, status_code, "text/json", Json::writeString(writer_, body));
    }

    void Response(const muzi::TcpConnectionPtr &conn, int status_code,
        const std::string &type, const std::string &body);
    
    void OnConnection(const muzi::TcpConnectionPtr &conn);

private:
    muzi::TcpServer server_;
    DBPtr db_;
    TablePtr user_tab_;
    KeyValue db_args_;
    Json::StreamWriterBuilder writer_;
    std::string md5_key_;
};

} // namespace ebo
