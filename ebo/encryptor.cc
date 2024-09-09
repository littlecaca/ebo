#include "encryptor.h"

#include <string>
#include <stdexcept>
#include <sstream>
#include <iomanip>

#include <openssl/md5.h>
#include <openssl/evp.h>



namespace ebo
{
std::string Encryptor::MD5(const std::string &text)
{
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    if (context == nullptr) 
    {
        throw std::runtime_error("Failed to create EVP_MD_CTX");
    }

    if (EVP_DigestInit_ex(context, EVP_md5(), nullptr) != 1) 
    {
        EVP_MD_CTX_free(context);
        throw std::runtime_error("Failed to initialize MD5 context");
    }

    if (EVP_DigestUpdate(context, text.c_str(), text.size()) != 1) 
    {
        EVP_MD_CTX_free(context);
        throw std::runtime_error("Failed to update MD5 hash");
    }

    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_length = 0;

    if (EVP_DigestFinal_ex(context, digest, &digest_length) != 1) 
    {
        EVP_MD_CTX_free(context);
        throw std::runtime_error("Failed to finalize MD5 hash");
    }

    EVP_MD_CTX_free(context);

    std::ostringstream oss;
    for (unsigned int i = 0; i < digest_length; ++i) 
    {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    }

    return oss.str();
}

}   // namespace ebo
