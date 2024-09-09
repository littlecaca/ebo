#pragma once

#include <algorithm>
#include <string>
#include <unordered_map>
#include <memory.h>
#include <memory>

#include "c_http_parser.h"
#include "logger.h"
#include "noncopyable.h"

namespace ebo
{
using HeaderMap = std::unordered_map<std::string, std::string>;

struct HttpMessage
{
public:
    HttpMessage()
        : parsed_(false), header_parsed_(false)
    {
        ::memset(&parser_, 0, sizeof parser_);
        http_parser_init(&parser_, HTTP_REQUEST);
        parser_.data = this;
    }

    virtual ~HttpMessage()
    {
    }

    size_t Parse(const std::string &raw_msg_str);

    bool IsMessageParsed() const { return parsed_; }
    bool IsHeaderParsed() const { return header_parsed_; }

    unsigned Method() const { return method_; }
    unsigned Status() const { return status_; }

    const std::string &MethodStr() const { return method_str_; }
    const std::string &StatusStr() const { return status_str_; }
    const std::string &Url() const { return url_; }
    const HeaderMap &Headers() const { return headers_; }
    const std::string &Body() const { return body_; }
    
private:
    static int OnMessageBegin(http_parser *);
    static int OnUrl(http_parser *, const char *at, size_t length);
    static int OnStatus(http_parser *, const char *at, size_t length);
    static int OnHeaderField(http_parser *, const char *at, size_t length);
    static int OnHeaderValue(http_parser *, const char *at, size_t length);
    static int OnHeadersComplete(http_parser *);
    static int OnBody(http_parser *, const char *at, size_t length);
    static int OnMessageComplete(http_parser *);

private:
    bool parsed_;
    bool header_parsed_;
    http_parser parser_;
    friend class SetSettings;
    static http_parser_settings settings_;
    std::string __last_field_;
    
private:
    unsigned method_;
    unsigned status_;
    std::string method_str_;
    std::string status_str_;

    std::string url_;
    HeaderMap headers_;
    std::string body_;
};

class Url
{
public:
    using UrlArgMap = std::unordered_map<std::string, std::string>;

    Url(const std::string raw_url_str)
        : parsed_(false), raw_url_str_(raw_url_str)
    {
        Parse();
    }

public:
    const std::string &GetRawUrl() const { return raw_url_str_; }
    
    std::string &operator[](const std::string &key)
    {
        return args_[key];
    }

    bool Find(const std::string &key) const
    {
        return args_.find(key) != args_.end();
    }

    std::string &At(const std::string &key)
    {
        return args_.at(key);
    }

    const std::string &At(const std::string &key) const
    {
        return args_.at(key);
    }

    bool IsValid() { return parsed_; }

    std::string &RawUrlStr() { return raw_url_str_; }
    std::string &Protocol() { return protocol_; }
    std::string &Host() { return host_; }
    unsigned Port() { return port_; }
    std::string &UrlPath() { return url_; }
    UrlArgMap &Args() { return args_; }

private:
    bool Parse();

private:
    bool parsed_;

    std::string raw_url_str_;
    std::string protocol_;
    std::string host_;
    unsigned port_;
    std::string url_;
    UrlArgMap args_;
};
}   // namespace ebo

