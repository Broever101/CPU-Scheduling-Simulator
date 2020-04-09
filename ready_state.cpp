#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include <sstream>
#include "Process.h"
#include <string.h>
#include <memory>

typedef std::vector<std::shared_ptr<Process>> p_vector;

void writeToPipe(int, std::string&);
std::string readFromPipe(int);
std::string createPacket(const std::shared_ptr<Process>&);
void createProcs(std::string data, p_vector&);

int main(int argc, char* argv[]){
    int new_ready = open("new2ready", O_RDONLY);
    if (new_ready < 0) std::cout<<"Could not open new2ready in ready.\n";
    int ready_running = open("ready2running", O_WRONLY);
    if (ready_running < 0) std::cout<<"Could not open ready2running in ready.\n";
    
    std::string scheduling_algo = readFromPipe(new_ready);
    p_vector procs;
    std::string packet;
    //size_t time = 0;
    int i = 0;
    std::string data = readFromPipe(new_ready);
    do {
        createProcs(data, procs);
        std::cout<<"PROCESS ARRIVED IN READY: "<<(*procs.rbegin())->proc_name<<std::endl;
        packet = createPacket(*(procs.begin()+i));
        writeToPipe(ready_running, packet);
        std::cout<<"PROCESS DISPATCHED TO RUNNING: "<<(*(procs.begin()+i))->proc_name<<std::endl;
        //procs.erase(procs.begin());
        data = readFromPipe(new_ready);
        ++i;
    }while (data != "closed" && !procs.empty());
    close(new_ready);
    close(ready_running);
    exit(0);
}

std::string createPacket(const std::shared_ptr<Process>& proc){
    return std::string(proc->proc_name + "\n" +
                std::to_string(proc->arrival) + "\n" +
                std::to_string(proc->burst));
}

void printVector(const p_vector& procs){
    for (auto i: procs){
        std::cout<<i->proc_name<<std::endl;
        std::cout<<i->arrival<<std::endl;
        std::cout<<i->burst<<std::endl;
    }
}

void createProcs(std::string data, p_vector& procs){
    std::istringstream stream(std::move(data));
    std::string proc; 
    size_t arr, burst;
    while(stream>>proc){
        stream>>arr;
        stream>>burst;
        procs.push_back(std::make_shared<Process>(proc, arr, burst));
    }
}

void writeToPipe(int pipe_fd, std::string& message){
    write(pipe_fd, message.c_str(), message.size()+1);
}

std::string readFromPipe(int pipe_fd){
    char msg[256];
    size_t bytes = read(pipe_fd, msg, sizeof(msg));
    if (bytes == 0) return "closed";
    msg[bytes] = '\0';
    return msg;
}