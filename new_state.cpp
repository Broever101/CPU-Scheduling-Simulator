#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

void writeToPipe(int, std::string&);

int main(int argc, char* argv[]){
    int new_ready = open("new2ready", O_WRONLY);
    if (new_ready < 0) std::cout<<"Could not open new2ready in new.\n";
    else{
        //std::cout<<"Writing to new2ready in new state.\n";
        std::string message = "SMOKE THIS PIPE READY!";
        writeToPipe(new_ready, message);
        close(new_ready);
    }  
}

void writeToPipe(int pipe_name, std::string& message){
    write(pipe_name, message.c_str(), message.size()+1);
}