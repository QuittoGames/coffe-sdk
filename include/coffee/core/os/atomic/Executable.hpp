# pragma once

#include"coffee/core/os/cmd/Command.hpp"
#include<string>
#include<vector>

class Executable : public Command{
protected:
    std::string shell;
    std::vector<std::string> command_raw{};
};
