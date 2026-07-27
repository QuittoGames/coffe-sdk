#include "../Process.hpp"
#include <string>
#include <vector>


class LinuxProcess : public Process {
public:
    int run(const std::string& executable,const std::vector<std::string>& args) override;
};
