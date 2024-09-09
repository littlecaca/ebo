#include "http_parser.h"
#include "logger.h"

using namespace muzi;
using namespace ebo;

int main(int argc, char const *argv[])
{
    LOG_INFO << "Start http_parser_test";

    Url url("http://192.168.154.129:6607/login?time=155422432&user=liming&salt=21113&sign=a6ed5cc0ad808b048ff01e2ed6bdb073");
    

    if (url.IsValid())
    {
        printf("protocol: %s\n", url.Protocol().data());
        printf("host: %s\n", url.Host().data());
        printf("port: %d\n", url.Port());
        printf("url: %s\n", url.UrlPath().data());

        if (url.Find("obj"))
        {
            printf("arg obj: %s\n", url["obj"].data());
        }
        for (auto &[key, val] : url.Args())
        {
            printf("(%s, %s)", key.data(), val.data());
        }
    }
    else
    {
        printf("not a valid url\n");
        return -1;
    }
    return 0;
}