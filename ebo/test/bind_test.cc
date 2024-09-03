
#include <string.h>
#include <stdio.h>

#include <sys/unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

int main(int argc, char const *argv[])
{
    /* code */
    int sock_fd = ::socket(PF_LOCAL, SOCK_STREAM, 0);
    sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    char curpath[256];
    ::getcwd(curpath, sizeof curpath);
    const char *path = "./log/server.sock";
    if (::access("log", W_OK | R_OK) < 0)
    {
        printf("not log found...\n");
        if (::mkdir("log", 0777) < 0)
        {
            ::perror("::mkdir() fails");
            return -1;
        }
    }

    ::strncpy(addr.sun_path, path, sizeof addr.sun_path);
    printf("%s\n", addr.sun_path);
    if (::bind(sock_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(sockaddr_un)) < 0)
    {
        ::perror("::bind() fails");
        return -2;
    }
    else
    {
        ::printf("%d\n", sock_fd);
    }

    return 0;
}
