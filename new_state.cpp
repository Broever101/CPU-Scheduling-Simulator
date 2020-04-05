#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <vector>

void writeToPipe(int, std::string&);
typedef std::vector<std::string> s_vector;

void split(const std::string &str, const char token,
           s_vector &tokens){
    size_t start = 0, end = 0;
    while (end != std::string::npos){
        end = str.find(token, start);
        tokens.push_back(str.substr(start, end - start));
        start = end + sizeof(token);
    }
}

int main(int argc, char* argv[]){
    //int new_ready = open("new2ready", O_WRONLY);
    //if (new_ready < 0) exit(1);
    s_vector tokens;
    std::string file_path = "processes/Sample_2_FCFS.txt";
    int fd = open(file_path.c_str(), O_RDONLY);
    if (fd < 0) perror("Error :");
    char kek[99];
    int bytes = read(fd, kek, sizeof(kek));
    split(kek, '\n', tokens);
    close(fd);
}



void writeToPipe(int pipe_name, std::string& message){
    write(pipe_name, message.c_str(), message.size()+1);
}