#include "log_outputer.h"

#include <functional>

#include <signal.h>
#include <sys/fcntl.h>

#include "event_loop.h"

using namespace std::placeholders;

namespace ebo
{
LogOupter::LogOupter(const std::string &base_name)
    : base_name_(base_name),
      started_(false),
      addr_(LogServer::kLocalPath),
      sock_client_(muzi::socket::CreateBlockingSockOrDie(addr_.GetProtoFamily())),
      process_(std::bind(&LogOupter::Run, this))
{
}

LogOupter::~LogOupter()
{
    if (started_)
    {
        Stop();
    }
}

void LogOupter::Start()
{
    started_ = true;

    // Start log server subprocess
    process_.CreateSubProcess();

    ::usleep(1000 * 10);    // 10ms
    // Start client
    int ret = sock_client_.Connect(addr_);

    if (ret < 0)
    {
        LOG_FATAL_U(muzi::gStderrLogger) 
            << "LogOupter::Start() Can not connect to log server";
    }
}

void LogOupter::Stop()
{
    process_.CloseFd();
    started_ = false;
}

void LogOupter::Output(const SmallBuffer &buf)
{
    sock_client_.Send(buf.data(), buf.size());
}

void LogOupter::Flush()
{
    // Do nothing
}

int LogOupter::Run()
{
    process_.SwitchToDaemon();
    
    // Close the log
    muzi::ClosedOuputer closed_outputer;
    muzi::gDefaultLogger.SetOutputer(&closed_outputer);

    DEBUGINFO << "Start";
    muzi::EventLoopThread loop_thread;
    muzi::EventLoop *loop = loop_thread.StartLoop();

    LogServer log_server(loop, base_name_);
    DEBUGINFO << "Construct LogServer";
    log_server.Start();
    DEBUGINFO << "Start LogServer";

    int fd;
    ::sleep(4);
    while (true)
    {
        fd = process_.RecvFd();
        DEBUGINFO << "recevice fd " << fd;
        if (fd <= 0)
        {
            DEBUGINFO << "I am done, received " << fd;
            break;
        }
    }
    DEBUGINFO << "Exit normally";
    return 0;
}

}   // namespace ebo
