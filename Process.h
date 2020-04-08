#include <iostream>
class Process{
public:
    std::string proc_name;
    size_t arrival;
    size_t burst;
    short priority = 0;
    inline Process(std::string proc, size_t arr, size_t burst) : proc_name(proc), arrival(arr), burst(burst) {}
};