#pragma once

#include <functional>

#include <memory.h>
#include <signal.h>
#include <unistd.h>

#include "noncopyable.h"

namespace ebo 
{
class SubProcess : muzi::noncopyable
{
public:
    using EntryFunc = std::function<int(void)>;

    SubProcess(EntryFunc func) :
        func_(std::move(func)),
        pid_(-1),
        sock_fd_(-1)
    {
    }
    
    /// @brief Sometimes, we need send `this` as the arg EntryFunc,
    /// so we must allow to construct without setting entryfunc.
    SubProcess()
        : SubProcess(EntryFunc())
    {
    }

    void SetEntryFunc(EntryFunc func)
    {
        func_ = std::move(func);
    }

    /// @brief Create sub process and return it's pid
    int CreateSubProcess();

    pid_t GetPid() const { return pid_; }

    /// @attention Can only called by main process,
    /// the argument fd must be a valid file descriptor
    void SendFd(int fd) const;

    /// @return -1 represents there is error happening 
    /// @attention Can only called by subprocess
    int RecvFd() const;

    static void SwitchToDaemon();

    void CloseFd()
    {
        ::close(sock_fd_);
    }

private:
    EntryFunc func_;
    // -1 represents the subprocess has not yet been created
    pid_t pid_;
    int sock_fd_;
};

}   // namespace ebo
