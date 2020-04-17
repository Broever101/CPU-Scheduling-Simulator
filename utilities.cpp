#include "utilities.h"
#include <unistd.h>

void utils::writeToPipe(int pipe_fd, std::string &message)
{
    write(pipe_fd, message.c_str(), message.size() + 1);
}

std::string utils::readFromPipe(int pipe_fd)
{
    char msg[256];
    size_t bytes = read(pipe_fd, msg, sizeof(msg));
    if (bytes == 0)
        return "closed";
    if (bytes == -1)
        return "empty";
    msg[bytes] = '\0';
    return msg;
}

std::string utils::createPacket(const process &proc)
{
    return std::string(proc->proc_name + "\n" +
                       std::to_string(proc->arrival) + "\n" +
                       std::to_string(proc->burst) + "\n" +
                       std::to_string(proc->remaining_burst) + "\n" +
                       std::to_string(proc->turnaround) + "\n" +
                       std::to_string(proc->waiting)) + "\n" + 
                       std::to_string(proc->block_type);
}

void utils::printVector(const p_vector &procs)
{
    for (auto i : procs)
    {
        std::cout << i->proc_name << std::endl;
        std::cout << i->arrival << std::endl;
        std::cout << i->burst << std::endl;
    }
}

void utils::setNonBlock(const int &fd)
{
    if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
    {
        perror("Error ");
        exit(0);
    }
}

std::string utils::readFromFile(const int &fd)
{
    char data[256];
    size_t bytes = read(fd, data, sizeof(data));
    data[bytes] = '\0';
    return std::move(data);
}

void utils::createProcs(std::string data, p_vector &procs)
{
    std::istringstream stream(std::move(data));
    std::string proc;
    size_t arr, burst, rem_burst, turn, wait, block;
    while (stream >> proc)
    {
        stream >> arr >> burst >> rem_burst >> turn >> wait>>block;
        procs.push_back(std::make_shared<Process>(proc, arr, burst, rem_burst, turn, wait, block));
    }
}

void utils::createProcs(std::string data, std::string &algorithm, std::string &time_quantum,
                        p_vector &procs)
{
    std::istringstream stream(std::move(data));
    stream >> algorithm;
    if (algorithm == "RR")
        stream >> time_quantum;
    std::string proc;
    size_t arr, burst;
    while (stream >> proc)
    {
        stream >> arr;
        stream >> burst;
        procs.push_back(std::make_shared<Process>(proc, arr, burst, burst, 0, 0, -1));
    }
}

void utils::createProc(std::string data, process &procs)
{
    std::istringstream stream(std::move(data));
    std::string proc;
    size_t arr, burst, rem_burst, turn, wait, block;
    stream >> proc >> arr >> burst >> rem_burst >> turn >> wait>>block;
    procs = std::make_shared<Process>(proc, arr, burst, rem_burst, turn, wait, block);
}