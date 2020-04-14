#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sstream>
#include <memory>
#include <time.h>
//#include <vector>
#include "Process.h"

typedef std::shared_ptr<Process> process;

std::string readFromPipe(int);
void writeToPipe(int, std::string&);
void createProc(std::string, process&);
std::string createPacket(const process&);

int main(int argc, char* argv[]){
    srand(time(NULL));
    process proc(nullptr);
    int ready_running = open("ready2running", O_RDONLY);
    if (ready_running < 0) std::cout<<"Could not open ready2running in running.\n";
    int running_block = open("running2block", O_WRONLY);
    if (running_block < 0) std::cout<<"Could not open running2block in running.\n"; 
    int running_ready = open("running2ready", O_WRONLY);
    if (running_ready < 0) std::cout<<"Could not open running2ready in running.\n";
    int running_exit = open("running2exit", O_WRONLY);

    size_t burst;
    bool block = false;
    std::string data = readFromPipe(ready_running);
    while (data != "closed"){
        createProc(data, proc);
        std::cout<<"RUNNING: "<<proc->proc_name<<" scheduled.\n";
        burst = 0;
        while (burst < proc->remaining_burst){
            sleep(1);
            burst++;
            if (burst == 5){
                block = rand() % 2;
                if (block){
                    proc->remaining_burst -= burst;
                    data = createPacket(proc);
                    writeToPipe(running_block, data);
                    std::cout<<"RUNNING: "<<proc->proc_name<<" blocked.\n";
                    break;
                }
            }
        }

        if (!block){
            proc->remaining_burst -= burst;
            data = createPacket(proc);
        }

        if (proc->remaining_burst == 0){
            writeToPipe(running_exit, data);
            std::cout<<"RUNNING: "<<proc->proc_name<<" exited.\n";
        }

        writeToPipe(running_ready, data);
        data = readFromPipe(ready_running);
    }
    close(ready_running);
}

void writeToPipe(int pipe_name, std::string& message){
    write(pipe_name, message.c_str(), message.size()+1);
}

void createProc(std::string data, process& procs){
    std::istringstream stream(std::move(data));
    std::string proc; 
    size_t arr, burst, rem_burst, turn, wait;
    stream>>proc>>arr>>burst>>rem_burst>>turn>>wait;
    procs = std::make_shared<Process>(proc, arr, burst, rem_burst, turn, wait);   
}

std::string createPacket(const process& proc){
    return std::string(proc->proc_name + "\n" +
                std::to_string(proc->arrival) + "\n" +
                std::to_string(proc->burst) + "\n" +
                std::to_string(proc->remaining_burst) + "\n" +
                std::to_string(proc->turnaround) + "\n" +
                std::to_string(proc->waiting));
}

std::string readFromPipe(int pipe_name){
    char msg[256];
    size_t bytes = read(pipe_name, msg, sizeof(msg));
    if (bytes == 0) return "closed";
    msg[bytes] = '\0';
    return msg;
}