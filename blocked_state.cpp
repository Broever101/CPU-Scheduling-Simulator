#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
//#include <fcntl.h>
#include <algorithm>
#include <memory>
#include <vector>
#include <sstream>
//#include "Process.h"
#include <string.h>
#include <time.h>
#include "utilities.h"

//void createProcs(std::string, p_vector &);
void updateProcs(p_vector &);
void releaseProc(const int &, p_vector &);
//std::string createPacket(const process &);
//std::string readFromPipe(int pipe_fd);
//void writeToPipe(int, std::string &);
//void setNonBlock(const int&);

int main(int argc, char *argv[])
{
    srand(time(NULL));
    int running_block = open("running2block", O_RDONLY);
    if (running_block < 0)
        std::cout << "Could not open running2block in block.\n";
    int block_ready = open("block2ready", O_WRONLY);
    if (block_ready < 0)
        std::cout << "Could not open block2ready in block.\n";

    utils::setNonBlock(running_block);
    utils::setNonBlock(block_ready);
    std::string data;
    p_vector blocked_queue;
    data = utils::readFromPipe(running_block);
    while (data != "closed")
    {
        if (data != "empty")
        {
            utils::createProcs(data, blocked_queue);
            std::cout << "BLOCKED: " << blocked_queue.rbegin()->get()->proc_name << " received.\n";
            blocked_queue.rbegin()->get()->block_time = rand() % 5 + 15;
            std::cout<<"BLOCKED: "<<blocked_queue.rbegin()->get()->block_time<<" block time for "<<
            blocked_queue.rbegin()->get()->proc_name<<std::endl;
        }
        sleep(1);
        updateProcs(blocked_queue);
        releaseProc(block_ready, blocked_queue);
        data = utils::readFromPipe(running_block);
    }
    std::cout<<"RUNNING CLOSED PIPE\n";
    close(running_block);
    close(block_ready);
    exit(0);
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
            packet = utils::createPacket(procs[i]);
            utils::writeToPipe(block_ready, packet);
            std::cout << "BLOCKED: " << procs[i]->proc_name << " returned to READY.\n";
            procs.erase(procs.begin() + i);
        }
    }
}
