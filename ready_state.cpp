#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

void writeToPipe(int, std::string&);
std::string readFromPipe(int);

int main(int argc, char* argv[]){
    int ready_new = open("new2ready", O_RDONLY);
    if (ready_new < 0) std::cout<<"Could not open new2ready in ready.\n";
    else{
        std::string str = readFromPipe(ready_new);
        std::cout<<str<<" -- received in ready.\n";
        close(ready_new);
    }
    int ready_running = open("ready2running", O_WRONLY);
    if (ready_running < 0) std::cout<<"Could not open ready2running in ready.\n";
    else{
        std::string str1 = "SMOKE THIS PIPE RUNNING!";
        writeToPipe(ready_running, str1);
        close(ready_running);
    }

    int running_ready = open("running2ready", O_RDONLY);
    if (running_ready < 0) std::cout<<"Could not open running2ready in ready.\n";
    else{
        std::string message = readFromPipe(running_ready);
        std::cout<<message<<" -- received in ready.\n";
        close(running_ready);
    } 
}

void writeToPipe(int pipe_name, std::string& message){
    write(pipe_name, message.c_str(), message.size()+1);
}

std::string readFromPipe(int pipe_name){
    char msg[256];
    read(pipe_name, msg, sizeof(msg));
    return msg;
}