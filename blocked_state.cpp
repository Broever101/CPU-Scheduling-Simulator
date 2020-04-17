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
#include <map>
#include "utilities.h"
#include <pthread.h>

pthread_t input_t, output_t, printer_t;
bool main_status;
//void createProcs(std::string, p_vector &);
void updateProcs(p_vector &);
void releaseProc(const int &, p_vector &);
void* blockedQueue(void*);
//std::string createPacket(const process &);
//std::string readFromPipe(int pipe_fd);
//void writeToPipe(int, std::string &);
//void setNonBlock(const int&);
struct ThreadDump{
    int& fd;
    bool main_status;
    p_vector& blocked_queue;
    ThreadDump(int& fd, bool status, p_vector& bq):
    fd(fd), main_status(status), blocked_queue(bq){}
};

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
    p_vector input, output, printer;
    std::map<short, p_vector*> blocked_queue = {
        {0, &input}, {1, &output}, {2, &printer}
    };

    main_status = false;
    ThreadDump input_dump(block_ready, main_status, *blocked_queue[0]);
    ThreadDump output_dump(block_ready, main_status, *blocked_queue[1]);
    ThreadDump printer_dump(block_ready, main_status, *blocked_queue[2]);

    pthread_create(&input_t, NULL, &blockedQueue, &input_dump);
    pthread_create(&output_t, NULL, &blockedQueue, &output_dump);
    pthread_create(&printer_t, NULL, &blockedQueue, &printer_dump);
    process proc;
    //p_vector blocked_queue;
    data = utils::readFromPipe(running_block);
    while (data != "closed")
    {
        if (data != "empty")
        {
            utils::createProc(data, proc);
            blocked_queue[proc->block_type]->push_back(proc);

            std::cout << "BLOCKED: " << proc->proc_name << " received.\n";
            blocked_queue[proc->block_type]->rbegin()->get()->block_time = rand() % 5 + 15;
            std::cout<<"BLOCKED: "<<blocked_queue[proc->block_type]->rbegin()->get()->block_time
            <<" block time for "<<proc->proc_name<<std::endl;
        }
        // sleep(1);
        // for (auto i: blocked_queue){
        //     updateProcs(i.second);
        //     releaseProc(block_ready, i.second);
        // }
        data = utils::readFromPipe(running_block);
    }
    std::cout<<"BLOCKED: RUNNING STATE CLOSED THE PIPE.\n";
    close(running_block);
    close(block_ready);
    main_status = true;
    exit(0);
}

void* blockedQueue(void* dump){
    ThreadDump* queue = (struct ThreadDump*) dump;
    while(!(queue->main_status && queue->blocked_queue.empty())){
        sleep(1);
        updateProcs(queue->blocked_queue);
        releaseProc(queue->fd, queue->blocked_queue);
    }
    std::cout<<"BLOCKED THREAD EXITING.\n";
    pthread_exit(NULL);
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
