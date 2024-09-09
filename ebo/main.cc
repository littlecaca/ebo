#include <iostream>
#include <fstream>

#include <jsoncpp/json/json.h>

#include "ebo_server.h"
#include "logger.h"
#include "event_loop.h"
#include "log_outputer.h"

int main(int argc, char const *argv[])
{
    std::cout << "Ebo Server Starting..." << std::endl;

    // Start log server
    ebo::LogOupter outputer("ebo");

    std::cout << "Start Log server..." << std::endl;
    outputer.Start();

    muzi::gDefaultLogger.SetOutputer(&outputer);

    muzi::EventLoop loop;
    ebo::EboServer server(&loop, muzi::InetAddress(6607), "ebo_server");
    server.Start();

    std::cout << "done" << std::endl;

    loop.Loop();

    return 0;
}
