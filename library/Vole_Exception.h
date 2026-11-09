#pragma once
#include <exception>
#include <string>

class VoleException : public std::exception {
private:
    std::string message;

public:
    VoleException(const std::string& msg) : message(msg) {}

    const char* what() const noexcept override {
        return message.c_str();
    }
};
