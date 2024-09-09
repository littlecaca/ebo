#pragma once

#include <string>


namespace ebo
{
class Encryptor 
{
public:
    Encryptor() = delete;
    ~Encryptor() = delete;

public:
    static std::string MD5(const std::string &text);
};
}   // namespace ebo
