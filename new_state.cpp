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

struct Process{
    std::string proc_name;
    size_t arrival;
    size_t burst;
    short priority = 0;
    Process(std::string proc, size_t arr, size_t burst):
    proc_name(proc), arrival(arr), burst(burst){}
};

typedef std::vector<std::string> s_vector;
typedef std::vector<Process*> p_vector;



void split(std::string data, s_vector& tokens){
    std::istringstream stream(data);
    std::string token;
    while (stream >> token)
        tokens.push_back(token);
}

void createProcs(std::string data, std::string& algorithm, p_vector& procs){
    std::istringstream stream(std::move(data));
    //std::cout<<stream.str()<<std::endl;
    stream >> algorithm;
    std::string proc; 
    size_t arr, burst;
    while(stream>>proc){
        //stream>>proc;
        stream>>arr;
        stream>>burst;
        //std::cout<<proc<<std::endl;
        procs.push_back(new Process(proc, arr, burst));
        //std::cout<<proc<<std::endl;
    }
    //std::cout<<procs[4]->proc_name<<std::endl;
}

int main(int argc, char* argv[]){
    //int new_ready = open("new2ready", O_WRONLY);
    //if (new_ready < 0) exit(1);
    s_vector tokens;
    std::string file_path = "processes/Sample_2_FCFS.txt";
    int fd = open(file_path.c_str(), O_RDONLY);
    if (fd < 0) perror("Error :");
    std::string data = readFromFile(fd);
    std::string algorithm;
    p_vector procs;
    createProcs(std::move(data), algorithm, procs);
    for (auto i : procs) std::cout<<i->proc_name<<"\n"<<i->arrival<<"\n"<<i->burst<<std::endl;
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