#include "log_outputer.h"

#include "logger.h"

using namespace ebo;

template <typename T>
constexpr T &&forward(typename std::remove_reference<T>::type &arg)
{
    return static_cast<T &&>(arg);
}

int main(int argc, char const *argv[])
{
    DEBUGINFO << "Start test";

    LogOupter outputer("test");
    outputer.Start();

    muzi::gDefaultLogger.SetOutputer(&outputer);

    LOG_INFO << "Hello, 好厉害！！！";
    LOG_INFO << "Hello, 好帮帮！！！";
    LOG_INFO << "Hello, 好拉拉！！！";
    LOG_INFO << "Hello, 好酷酷！！！";
    return 0;
}
