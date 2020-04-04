#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char* argv[]){
    int ready_new = open("new2ready", O_RDONLY);
    if (ready_new < 0) std::cout<<"Could not open new2ready in ready.\n";
    else{
        char str[256];
        read(ready_new, str, sizeof(str));
        std::cout<<str<<" -- received in ready.\n";
        close(ready_new);
    }
    int ready_running = open("ready2running", O_WRONLY);
    if (ready_running < 0) std::cout<<"Could not open ready2running in ready.\n";
    else{
        char str1[256] = "SMOKE THIS PIPE RUNNING!";
        write(ready_running, str1, sizeof(str1));
        close(ready_running);
    }
}