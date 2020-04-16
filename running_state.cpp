#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sstream>
#include <memory>
#include <time.h>
#include <utility>
#include "utilities.h"

int main(int argc, char *argv[])
{
    srand(time(NULL));
    process proc(nullptr);
    int ready_running = open("ready2running", O_RDONLY);
    if (ready_running < 0)
        std::cout << "Could not open ready2running in running.\n";
    int running_block = open("running2block", O_WRONLY);
    if (running_block < 0)
        std::cout << "Could not open running2block in running.\n";
    int running_ready = open("running2ready", O_WRONLY);
    if (running_ready < 0)
        std::cout << "Could not open running2ready in running.\n";
    int running_exit = open("running2exit", O_WRONLY);

    int time_quantum = std::stoi(utils::readFromPipe(ready_running));

    size_t burst, remaining_burst;
    bool block;
    //setNonBlock(ready_running);
    std::string data = utils::readFromPipe(ready_running);

    while (data != "closed")
    {
        utils::createProc(data, proc);
        std::cout << "RUNNING: " << proc->proc_name << " scheduled.\n";
        burst = 0;

        if (time_quantum == -1)
            remaining_burst = proc->remaining_burst;

        while (burst < remaining_burst)
        {
            sleep(1);
            burst++;
            block = false;
            if (burst == 5)
            {
                block = rand() % 2;
                if (block)
                {
                    proc->remaining_burst -= burst;
                    data = utils::createPacket(proc);
                    utils::writeToPipe(running_block, data);
                    std::cout << "RUNNING: " << proc->proc_name << " BLOCKED.\n";
                    break;
                }
            }
        }

        if (!block)
        {
            proc->remaining_burst -= burst;
            data = utils::createPacket(proc);
            utils::writeToPipe(running_ready, data);
            if (proc->remaining_burst == 0)
            {
                utils::writeToPipe(running_exit, data);
                std::cout << "RUNNING: " << proc->proc_name << " EXITED.\n";
            }
        }
        else
        {
            data = "NEXT";
            utils::writeToPipe(running_ready, data);
        }
        data = utils::readFromPipe(ready_running);
    }
    std::cout << "READY STATE CLOSED THE PIPE.\n";
    close(ready_running);
    close(running_block);
    close(running_exit);
    close(running_ready);
    exit(0);
}
