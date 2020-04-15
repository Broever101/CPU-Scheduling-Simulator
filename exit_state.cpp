#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "Process.h"
#include <memory>
#include <sstream>
#include <fcntl.h>

typedef std::shared_ptr<Process> process;
void createProc(std::string, process&);
std::string createPacket(const process&);
std::string readFromPipe(int);


int main(int argc, char *argv[])
{
    int running_exit = open("running2exit", O_RDONLY);
    if (running_exit < 0)
        std::cout << "Could not open running2exit in exit.\n";

    process proc(nullptr);
    std::string data = readFromPipe(running_exit);
    while (data != "closed")
    {
        createProc(data, proc);
        std::cout << "EXIT: " << proc->proc_name << " exited.\n";
        data = readFromPipe(running_exit);
    }
    close(running_exit);
    exit(0);
}

void createProc(std::string data, process &procs)
{
    std::istringstream stream(std::move(data));
    std::string proc;
    size_t arr, burst, rem_burst, turn, wait;
    stream >> proc >> arr >> burst >> rem_burst >> turn >> wait;
    procs = std::make_shared<Process>(proc, arr, burst, rem_burst, turn, wait);
}

std::string createPacket(const process &proc)
{
    return std::string(proc->proc_name + "\n" +
                       std::to_string(proc->arrival) + "\n" +
                       std::to_string(proc->burst) + "\n" +
                       std::to_string(proc->remaining_burst) + "\n" +
                       std::to_string(proc->turnaround) + "\n" +
                       std::to_string(proc->waiting));
}

std::string readFromPipe(int pipe_name)
{
    char msg[256];
    size_t bytes = read(pipe_name, msg, sizeof(msg));
    if (bytes == 0)
        return "closed";
    msg[bytes] = '\0';
    return msg;
}