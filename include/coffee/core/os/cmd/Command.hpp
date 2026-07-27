#pragma once

#include <string>
#include <vector>

class Command {
public:
    virtual ~Command() = default;

    virtual std::string name() const = 0;
    virtual void execute() = 0;
};
