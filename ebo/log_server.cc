#include "log_server.h"

#include <functional>

using namespace std::placeholders;

namespace ebo
{
const char *LogServer::kLocalPath = "./log/log.sock";

LogServer::LogServer(muzi::EventLoop *loop, const std::string &base_name)
    : addr_(kLocalPath),
      server_(loop, addr_, base_name),
      file_(base_name),
      write_count_(0)
{
    server_.SetMessageCallback(std::bind(&LogServer::OnMessage, this, _1, _2, _3));
    server_.SetConnectionCallback(std::bind(&LogServer::OnConnection, this, _1));
}

void LogServer::OnMessage(const muzi::TcpConnectionPtr &conn,
                        muzi::Buffer *buf, muzi::Timestamp time)
{
    file_.Append(buf->RetriveAllAsString());
    write_count_ = (write_count_ + 1) % kWriteCntToFlush;
    if (write_count_ == 0)
    {
        file_.Flush();
    }
}

void LogServer::OnConnection(const muzi::TcpConnectionPtr &conn)
{
    if (conn->IsDisConnected())
    {
        file_.Flush();
    }
}

}   // namespace ebo
