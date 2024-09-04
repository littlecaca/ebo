#include "http_parser.h"
#include "logger.h"

using namespace muzi;
using namespace ebo;

int main(int argc, char const *argv[])
{
    LOG_INFO << "Start http_parser_test";

    Url url("https://www.bilibili.com:8080/video/BV1rvWpeDEDi/?spm_id_from=333.1007.tianma.1-2-2.click&vd_source=f9450b608b4b6275a87a66573d9fc7fe");
    

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