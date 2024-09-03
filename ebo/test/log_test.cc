#include "log_outputer.h"

#include "logger.h"

using namespace ebo;

int main(int argc, char const *argv[])
{
    DEBUGINFO << "Start test";

    LogOupter outputer("test");
    outputer.Start();

    muzi::gStdioLogger.SetOutputer(&outputer);

    LOG_INFO_U(muzi::gStdioLogger) << "Hello, 我的强大，真的很大";
    LOG_INFO_U(muzi::gStdioLogger) << "Hello, 我的强大，真的很大";
    LOG_INFO_U(muzi::gStdioLogger) << "Hello, 我的强大，真的很大";

    return 0;
}
