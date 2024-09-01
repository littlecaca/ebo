#include "subprocess.h"

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include "current_thread.h"
#include "logger.h"

namespace ebo
{
int SubProcess::CreateSubProcess()
{
    if (!func_)
    {
        LOG_FATAL << "SubProcess::CreateSubProcess() func_ is empty";
    }
    int pipes[2] = {0};
    int ret = ::socketpair(AF_LOCAL, SOCK_STREAM, 0, pipes);

    if (ret == -1)
    {
        LOG_SYSFAT << "::socketpair() fails";
    }
    pid_t pid = ::fork();
    if (pid == -1)
    {
        LOG_SYSFAT << "::fork() fails";
    }

    if (pid == 0) {
        // Subprocess
        muzi::current_thread::ResetTid();
        ::close(pipes[1]);
        sock_fd_ = pipes[0];
        ret = func_();
        (void)ret;
        exit(0);
    }
    // Main Process
    ::close(pipes[0]);
    sock_fd_ = pipes[1];
    return pid_ = pid;
}

void SubProcess::SendFd(int fd) const
{
    msghdr msg = { 0 };
    // fill data info,
    // indeed, we do not need to send any data.
    iovec iov[1];
    char buf[] = "ebo fd";
    iov[0].iov_base = buf;
    iov[0].iov_len = sizeof buf;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    // fill control info.
    char cmsgbuf[CMSG_LEN(sizeof(int))] = { 0 };
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = sizeof cmsgbuf;

    cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    *reinterpret_cast<int *>(CMSG_DATA(cmsg)) = fd;

    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;   // mark sending file descriptors

    ssize_t ret = ::sendmsg(sock_fd_, &msg, 0);
    if (ret == -1) 
    {
        LOG_SYSERR << "::sendmsg() fails";
    }
}

int SubProcess::RecvFd() const
{
    msghdr msg = { 0 };
    // fill data info,
    // indeed, we do not need to send any data.
    iovec iov[1];
    char buf[32];
    iov[0].iov_base = buf;
    iov[0].iov_len = sizeof buf;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    // fill control info.
    /// @attention Must set it to 0
    char cmsgbuf[CMSG_LEN(sizeof(int))] = { 0 };
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = sizeof cmsgbuf;

    cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;   // mark sending file descriptors

    ssize_t ret = ::recvmsg(sock_fd_, &msg, 0);

    if (ret == -1) 
    {
        LOG_SYSERR << "::recvmsg() fails";
        return -1;
    }
    return *reinterpret_cast<int *>(CMSG_DATA(cmsg));
}

void SubProcess::SwitchToDaemon()
{
    int pid = ::fork();
    if (pid == -1)
    {
        LOG_SYSFAT << "::fork() fails";
    }
    if (pid > 0) exit(0);
    
    // subprocess1
    ::setsid();
    pid = ::fork();
    if (pid == -1)
    {
        LOG_SYSFAT << "::fork() fails";
    }
    if (pid > 0) exit(0);

    // user file creation mode mask
    ::umask(0);

    // subprocess2
    for (int i = 0; i < 3; ++i)
        ::close(i);
    // redirect
    int fd1 = open("/dev/null", O_RDWR);    // stdin
    int fd2 = dup(0);                       // stdout
    int fd3 = dup(0);                       // stderr
    if (fd1 != 0 || fd2 != 2 || fd3 != 3)
    {
        LOG_ERROR << "Can not redirect stdin, stdout, and stderr";
    }
}

}   // namespace ebo
