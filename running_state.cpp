#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char* argv[]){
    int running_ready = open("ready2running", O_RDONLY);
    if (running_ready < 0) std::cout<<"Could not open ready2running in running.\n";
    else{
        char str[256];
        read(running_ready, str, sizeof(str));
        std::cout<<str<<" -- received in running.\n";
        close(running_ready);
    }
}