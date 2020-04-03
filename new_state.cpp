#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char* argv[]){
    int new_ready = open("new2ready", O_WRONLY);
    if (new_ready < 0) std::cout<<"Could not open new2ready in new.\n";
    else{
        std::cout<<"Writing to new2ready in new state.\n";
        char str[256] = "SMOKE THIS PIPE READY";
        write(new_ready, str, sizeof(str));
        close(new_ready);
    }  
}