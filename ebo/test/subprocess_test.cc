#include "subprocess.h"

#include <logger.h>
#include <sys/time.h>

using namespace ebo;
using namespace muzi;

int ProcessEntry(SubProcess *process)
{
    int fd;
    while ((fd = process->RecvFd()) > 0)
    {
        LOG_INFO << "Get fd " << fd;
    }
    LOG_INFO << "Get fd " << fd << ", I am done";
    return 0;
}

int main(int argc, char const *argv[])
{
    LOG_INFO << "subprocess_test start...";
    
    SubProcess sub;
    sub.SetEntryFunc(std::bind(&ProcessEntry, &sub));
    sub.CreateSubProcess();

    sub.SendFd(2);
    sub.SendFd(1);

    usleep(1000 * 10);  // 10ms
    sub.SendFd(-1);
    return 0;
}

