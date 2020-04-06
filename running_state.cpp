#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


std::string readFromPipe(int);
void writeToPipe(int, std::string&);

int main(int argc, char* argv[]){
    int ready_running = open("ready2running", O_RDONLY);
    if (ready_running < 0) std::cout<<"Could not open ready2running in running.\n";
    else{
        std::string message = readFromPipe(ready_running);
        std::cout<<message<<" -- received in running.\n";
        close(ready_running);
    }

    int running_ready = open("running2ready", O_WRONLY);
    if (running_ready < 0) std::cout<<"Could not open running2ready in running.\n";
    else{
        std::string message = "SMOKE THIS PIPE READY (from running)";
        writeToPipe(running_ready, message);
        close(running_ready);
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