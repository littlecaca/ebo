#pragma once

#include <atomic>
#include <string>

#include "address.h"
#include "event_loop_thread.h"
#include "outputer.h"
#include "local_address.h"
#include "socket.h"
#include "subprocess.h"
#include "log_server.h"

namespace ebo
{
class LogOupter : public muzi::Outputer
{
public:
    LogOupter(const std::string &base_name);

    ~LogOupter();

    void Start();

    void Stop();

    bool IsStarted() const { return started_; }

public:
    void Output(const SmallBuffer &buf) override;

    void Flush() override;

private:
    // Subprocess main body
    int Run();

private:
    std::string base_name_;
    bool started_;

    muzi::LocalAddress addr_;
    muzi::Socket sock_client_;

    SubProcess process_;
};

}   // namespace ebo
