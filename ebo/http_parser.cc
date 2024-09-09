#include "http_parser.h"

namespace ebo
{

http_parser_settings HttpMessage::settings_;

class SetSettings
{
public:
    SetSettings()
    {
        HttpMessage::settings_.on_message_begin = &HttpMessage::OnMessageBegin;
        HttpMessage::settings_.on_url = &HttpMessage::OnUrl;
        HttpMessage::settings_.on_status = &HttpMessage::OnStatus;
        HttpMessage::settings_.on_header_field = &HttpMessage::OnHeaderField;
        HttpMessage::settings_.on_header_value = &HttpMessage::OnHeaderValue;
        HttpMessage::settings_.on_headers_complete = &HttpMessage::OnHeadersComplete;
        HttpMessage::settings_.on_body = &HttpMessage::OnBody;
        HttpMessage::settings_.on_message_complete = &HttpMessage::OnMessageComplete;
    }
};

namespace 
{
constexpr HttpMessage *CastToMsg(http_parser *parser)
{
    return static_cast<HttpMessage *>(parser->data);
}
SetSettings __set;

inline void ReportHttpError(http_parser *parser, const char *reporter)
{
    LOG_ERROR << reporter << " fails: (http_errno = " 
                << http_errno_name(static_cast<http_errno>(parser->http_errno))
                << "[" << parser->http_errno << "]) "
                <<  http_errno_description(static_cast<http_errno>(parser->http_errno));
}
}   // internal linkage


int HttpMessage::OnMessageBegin(http_parser *parser)
{
    return 0;
}

int HttpMessage::OnUrl(http_parser *parser, const char *at, size_t length)
{
    CastToMsg(parser)->url_.assign(at, length);
    return 0;
}

int HttpMessage::OnStatus(http_parser *parser, const char *at, size_t length)
{
    CastToMsg(parser)->status_str_.assign(at, length);
    return 0;
}

int HttpMessage::OnHeaderField(http_parser *parser, const char * at, size_t length)
{
    CastToMsg(parser)->__last_field_.assign(at, length);
    return 0;
}

int HttpMessage::OnHeaderValue(http_parser *parser, const char *at, size_t length)
{
    CastToMsg(parser)->headers_[CastToMsg(parser)->__last_field_].assign(at, length);
    return 0;
}

int HttpMessage::OnHeadersComplete(http_parser *parser)
{
    CastToMsg(parser)->header_parsed_ = true;
    return 0;
}

int HttpMessage::OnBody(http_parser *parser, const char *at, size_t length)
{
    CastToMsg(parser)->body_.assign(at, length);
    return  0;
}

int HttpMessage::OnMessageComplete(http_parser *parser)
{
    CastToMsg(parser)->status_ = parser->status_code;
    CastToMsg(parser)->method_ = parser->method;
    CastToMsg(parser)->method_str_ 
        = http_method_str(static_cast<http_method>(parser->method));
    CastToMsg(parser)->parsed_ = true;
    return 0;
}

size_t HttpMessage::Parse(const std::string &raw_msg_str)
{
    parsed_ = false;
    header_parsed_ = false;
    
    size_t ret = http_parser_execute(&parser_, &settings_, raw_msg_str.data(), raw_msg_str.size());
    if (ret < raw_msg_str.size())
    {
        if (parser_.http_errno != http_errno::HPE_OK)
        {
            ReportHttpError(&parser_, "HttpMessage::Parse(): http_parser_execute()");
            return false;
        }
    }
    return ret;
}

bool Url::Parse()
{
    // protocol ://
    auto protocol_end = raw_url_str_.find_first_of("://", 0);
    if (protocol_end == raw_url_str_.npos) return parsed_ = false;
    protocol_ = raw_url_str_.substr(0, protocol_end - 0);

    // host /
    auto host_start = protocol_end + 3;
    auto host_end = raw_url_str_.find_first_of('/', host_start);
    if (host_end == raw_url_str_.npos) return parsed_ = false;
    host_ = raw_url_str_.substr(host_start, host_end - host_start);

    // port :
    // DEBUGINFO << host_;
    auto host_real_end = host_.find_last_of(':');
    if (host_real_end != host_.npos && host_real_end < host_end - 1)
    {
        DEBUGINFO << host_.substr(host_real_end + 1);
        for (auto i = host_real_end + 1; i != host_.size(); ++i)
        {
            if (!std::isdigit(host_[i]))
                return parsed_ = false;
        }
        port_ = std::stoi(host_.substr(host_real_end + 1));
        host_.resize(host_real_end);
    }

    // url ?
    auto url_start = host_end;
    auto url_end = raw_url_str_.find_first_of('?', url_start);
    if (url_end == raw_url_str_.npos) url_end = raw_url_str_.size();

    // fill
    url_ = raw_url_str_.substr(url_start, url_end - url_start);

    if (url_end == raw_url_str_.size())
        return parsed_ = true;

    auto arg_start = url_end + 1;
    std::string key;

    while (arg_start < raw_url_str_.size())
    {
        auto arg_end = raw_url_str_.find_first_of('=', arg_start);
        if (arg_end == raw_url_str_.npos)
            break;
        key = raw_url_str_.substr(arg_start, arg_end - arg_start);
        auto value_start = arg_end + 1;
        auto value_end = raw_url_str_.find_first_of('&', value_start);
        if (value_end == raw_url_str_.npos)
        {
            value_end = raw_url_str_.size();
        }
        args_[key] = raw_url_str_.substr(value_start, value_end - value_start);
        arg_start = value_end + 1;
    }
    return parsed_ = true;
}

} // namespace ebo
