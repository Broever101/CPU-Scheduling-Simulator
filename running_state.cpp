#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sstream>
#include <memory>
//#include <vector>
#include "Process.h"

typedef std::shared_ptr<Process> process;

std::string readFromPipe(int);
void writeToPipe(int, std::string&);
void createProc(std::string, process&);

int main(int argc, char* argv[]){
    process proc(nullptr);
    int ready_running = open("ready2running", O_RDONLY);
    if (ready_running < 0) std::cout<<"Could not open ready2running in running.\n";
    
    std::string data = readFromPipe(ready_running);
    while (data != "closed"){
        createProc(data, proc);
        std::cout<<"PROCESS SCHEDULED IN RUNNING: "<<proc->proc_name<<std::endl;
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
    size_t arr, burst;
    stream>>proc>>arr>>burst;
    procs = std::make_shared<Process>(proc, arr, burst);   
}

std::string readFromPipe(int pipe_name){
    char msg[256];
    size_t bytes = read(pipe_name, msg, sizeof(msg));
    if (bytes == 0) return "closed";
    msg[bytes] = '\0';
    return msg;
}