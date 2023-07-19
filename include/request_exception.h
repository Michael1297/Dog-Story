#pragma once
#include <string>
#include <exception>
#include <utility>

class RequestException : public std::exception{
public:
    RequestException(std::string error_code, std::string error_message)
            : code_(std::move(error_code))
            , message_(std::move(error_message))
            {}

    const char* what() const noexcept override { return  code_.c_str(); }
    const auto& message() const noexcept { return  message_; }
private:
    std::string code_, message_;
};
