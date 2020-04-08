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

typedef std::vector<Process*> p_vector;

void writeToPipe(int, std::string&);
std::string readFromPipe(int);
void createProcs(std::string data, p_vector&);

int main(int argc, char* argv[]){
    int new_ready = open("new2ready", O_RDONLY);
    if (new_ready < 0) std::cout<<"Could not open new2ready in ready.\n";
    
    std::string scheduling_algo = readFromPipe(new_ready);
    p_vector procs;
    std::string data = readFromPipe(new_ready);
    while (data != "closed"){
        createProcs(data, procs);
        std::cout<<"Process scheduled:\n";
        std::cout<<(*procs.rbegin())->proc_name<<std::endl;
        data = readFromPipe(new_ready);
    }
    std::cout<<"CLOSED.\n";
    close(new_ready);
    exit(0);
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
        procs.push_back(new Process(proc, arr, burst));
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