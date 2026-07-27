# pragma once

#include"coffee/core/os/cmd/Command.hpp"
#include<string>
#include<vector>

class Process {
public:
    virtual ~Process() = default;

    virtual int run(
        const std::string& executable,
        const std::vector<std::string>& args
    ) = 0;
};
