#pragma once

#include <string>

#include "noncopyable.h"
#include "local_address.h"
#include "event_loop.h"
#include "tcp_server.h"
#include "log_file.h"

namespace ebo
{

// Created and run in the subprocess
class LogServer : muzi::noncopyable
{
public:
    static const char *kLocalPath;

    LogServer(muzi::EventLoop *loop, const std::string &base_name);

    ~LogServer()
    {
        ::unlink(kLocalPath);
    }

    void Start()
    {
        server_.Start();
    }

private:
    void OnMessage(const muzi::TcpConnectionPtr &conn, 
        muzi::Buffer *buf, muzi::Timestamp time);

    void OnConnection(const muzi::TcpConnectionPtr &conn);

private:
    muzi::LocalAddress addr_;
    muzi::TcpServer server_;
    muzi::LogFile file_;

    int write_count_;
    static constexpr size_t kWriteCntToFlush = 2;
};
    

}   // namespace ebo
