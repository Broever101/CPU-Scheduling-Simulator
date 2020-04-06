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
#include <sstream>

std::string readFromFile(const int&);
void writeToPipe(int, std::string&);

typedef std::vector<std::string> s_vector;


void split(const std::string& data, s_vector& tokens){
    std::istringstream stream(std::move(data));
    std::string token;
    while (stream >> token)
        tokens.push_back(token);
}

int main(int argc, char* argv[]){
    //int new_ready = open("new2ready", O_WRONLY);
    //if (new_ready < 0) exit(1);
    s_vector tokens;
    std::string file_path = "processes/Sample_2_FCFS.txt";
    int fd = open(file_path.c_str(), O_RDONLY);
    if (fd < 0) perror("Error :");
    std::string data = readFromFile(fd);
    std::cout<<data;
    split(data, tokens);
    close(fd);
}

std::string readFromFile(const int& fd){
    char data[256];
    size_t bytes = read(fd, data, sizeof(data));
    data[bytes] = '\0';
    return std::move(data);
}

void writeToPipe(int pipe_name, std::string& message){
    write(pipe_name, message.c_str(), message.size()+1);
}