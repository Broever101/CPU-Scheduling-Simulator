#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fcntl.h>
#include <algorithm>
#include <memory>
#include <vector>
#include <sstream>
#include "Process.h"
#include <string.h>
#include <time.h>

typedef std::vector<std::shared_ptr<Process>> p_vector;
typedef std::shared_ptr<Process> process;

void createProcs(std::string, p_vector &);
void updateProcs(p_vector &);
void releaseProc(const int &, p_vector &);
std::string createPacket(const process &);
std::string readFromPipe(int pipe_fd);
void writeToPipe(int, std::string &);
void setNonBlock(const int&);

int main(int argc, char *argv[])
{
    srand(time(NULL));
    int running_block = open("running2block", O_RDONLY);
    if (running_block < 0)
        std::cout << "Could not open running2block in block.\n";
    int block_ready = open("block2ready", O_WRONLY);
    if (block_ready < 0)
        std::cout << "Could not open block2ready in block.\n";

    setNonBlock(running_block);
    setNonBlock(block_ready);
    std::string data;
    p_vector blocked_queue;
    data = readFromPipe(running_block);
    while (data != "closed")
    {
        if (data != "empty")
        {
            createProcs(data, blocked_queue);
            std::cout << "BLOCKED: " << blocked_queue.rbegin()->get()->proc_name << " received.\n";
            blocked_queue.rbegin()->get()->block_time = rand() % 5 + 15;
        }
        sleep(1);
        updateProcs(blocked_queue);
        releaseProc(block_ready, blocked_queue);
        data = readFromPipe(running_block);
    }
}

void createProcs(std::string data, p_vector &procs)
{
    std::istringstream stream(std::move(data));
    std::string proc;
    size_t arr, burst, rem_burst, turn, wait;
    while (stream >> proc)
    {
        stream >> arr >> burst >> rem_burst >> turn >> wait;
        procs.push_back(std::make_shared<Process>(proc, arr, burst, rem_burst, turn, wait));
    }
}

void updateProcs(p_vector &procs)
{
    for (auto i : procs)
    {
        i->turnaround++;
        i->block_time--;
    }
}

void releaseProc(const int &block_ready, p_vector &procs)
{
    std::string packet;
    for (auto i = 0; i < procs.size(); ++i)
    {
        if (procs[i]->block_time == 0)
        {
            packet = createPacket(procs[i]);
            writeToPipe(block_ready, packet);
            std::cout << "BLOCKED: " << procs[i]->proc_name << " returned to ready.\n";
            procs.erase(procs.begin() + i);
        }
    }
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

void setNonBlock(const int& fd){
    if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
    {
        perror("Error ");
        exit(0);
    }
}

std::string readFromPipe(int pipe_fd)
{
    char msg[256];
    size_t bytes = read(pipe_fd, msg, sizeof(msg));
    if (bytes == 0)
        return "closed";
    if (bytes < 0)
        return "empty";
    msg[bytes] = '\0';
    return msg;
}

void writeToPipe(int pipe_fd, std::string &message)
{
    write(pipe_fd, message.c_str(), message.size() + 1);
}