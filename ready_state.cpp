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
    int ready_new = open("new2ready", O_RDONLY);
    if (ready_new < 0) std::cout<<"Could not open new2ready in ready.\n";
    else{
        std::string str = readFromPipe(ready_new);
        std::cout<<str<<" -- received in ready.\n";
        str = readFromPipe(ready_new);
        p_vector procs;
        createProcs(str, procs);
        str = readFromPipe(ready_new);
        createProcs(str, procs);
        std::cout<<"Procs received in ready:\n";
        for (auto i: procs){
            std::cout<<i->proc_name<<std::endl;
            std::cout<<i->arrival<<std::endl;
            std::cout<<i->burst<<std::endl;
        }
        close(ready_new);
    }
    // int ready_running = open("ready2running", O_WRONLY);
    // if (ready_running < 0) std::cout<<"Could not open ready2running in ready.\n";
    // else{
    //     std::string str1 = "SMOKE THIS PIPE RUNNING!";
    //     writeToPipe(ready_running, str1);
    //     close(ready_running);
    // }

    // int running_ready = open("running2ready", O_RDONLY);
    // if (running_ready < 0) std::cout<<"Could not open running2ready in ready.\n";
    // else{
    //     std::string message = readFromPipe(running_ready);
    //     std::cout<<message<<" -- received in ready.\n";
    //     close(running_ready);
    // } 
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

void writeToPipe(int pipe_name, std::string& message){
    write(pipe_name, message.c_str(), message.size()+1);
}

std::string readFromPipe(int pipe_name){
    char msg[256];
    size_t bytes = read(pipe_name, msg, sizeof(msg));
    msg[bytes] = '\0';
    return msg;
}