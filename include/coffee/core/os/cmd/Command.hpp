#pragma once

#include <string>

class Command {
public:
    virtual ~Command() = default;
    virtual std::string name() const = 0;
    virtual bool can_execute() const { return true; }
    virtual void execute() = 0;
};
