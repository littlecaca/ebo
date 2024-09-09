#include "encryptor.h"

#include <string>

#include "logger.h"

int main(int argc, char const *argv[])
{
    LOG_INFO << "encryptor_test Start";

    std::string text = "你是谁呀";
    LOG_INFO << "expect: 5dad7e6fd54af78d4ce2c4741d0878e4 " 
             << "get: " << ebo::Encryptor::MD5(text);
    return 0;
}
