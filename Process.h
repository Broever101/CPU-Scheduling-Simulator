#include <iostream>
class Process
{
public:
    std::string proc_name;
    size_t arrival;
    size_t burst;
    size_t remaining_burst;
    size_t turnaround;
    size_t waiting;
    short priority = 0;
    short block_time = 0;
    inline Process(std::string proc, size_t arr, size_t burst, size_t rem_burst, size_t turn, size_t wait) : proc_name(proc), arrival(arr), burst(burst), remaining_burst(rem_burst), turnaround(turn), waiting(wait) {}
    bool operator==(const Process &obj)
    {
        return this->proc_name == obj.proc_name;
    }
};