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
#include <algorithm>
#include "Process.h"


typedef std::vector<Process*> p_vector;

std::string readFromFile(const int&);
void writeToPipe(int, std::string&);
std::string createPacket(Process*);
void createProcs(std::string, std::string&, p_vector&);
void printVector(const p_vector&);

int main(int argc, char* argv[]){
    p_vector procs;
    std::string scheduling_algo;
    std::string file_path = "processes/Sample_2_FCFS.txt";
    int proc_file = open(file_path.c_str(), O_RDONLY);
    if (proc_file < 0) {
        std::cout<<file_path<<std::endl;
        perror("Error ");
    }
    int new_ready = open("new2ready", O_WRONLY);
    if (new_ready < 0) {
        std::cout<<"new2ready"<<std::endl;
        perror("Error ");
    }
    
    std::string data = readFromFile(proc_file);
    close(proc_file);

    createProcs(std::move(data), scheduling_algo, procs);

    //printVector(procs);

    std::stable_sort(procs.begin(), procs.end(),
    [] (auto proc1, auto proc2){
        if (proc1->arrival < proc2->arrival)
            return true;
        else if (proc1->proc_name < proc2->proc_name)
            return true;
    }); 

    printVector(procs);       

    writeToPipe(new_ready, scheduling_algo);

    size_t time = 0;
    for (auto i: procs){
        while(time != i->arrival){
            sleep(1);
            ++time;
        }
        std::string packet = createPacket(i);
        writeToPipe(new_ready, packet);
    }
    std::cout<<"NEW CLOSED.\n";
    close(new_ready);
}

std::string readFromFile(const int& fd){
    char data[256];
    size_t bytes = read(fd, data, sizeof(data));
    data[bytes] = '\0';
    return std::move(data);
}

void printVector(const p_vector& procs){
    for (auto i: procs){
        std::cout<<i->proc_name<<std::endl;
        std::cout<<i->arrival<<std::endl;
        std::cout<<i->burst<<std::endl;
    }
}

std::string createPacket(Process* proc){
    return std::string(proc->proc_name + "\n" +
                std::to_string(proc->arrival) + "\n" +
                std::to_string(proc->burst));
}

void writeToPipe(int pipe_fd, std::string& message){
    write(pipe_fd, message.c_str(), message.size()+1);
}

void createProcs(std::string data, std::string& algorithm, 
                p_vector& procs){
    std::istringstream stream(std::move(data));
    stream >> algorithm;
    std::string proc; 
    size_t arr, burst;
    while(stream>>proc){
        stream>>arr;
        stream>>burst;
        procs.push_back(new Process(proc, arr, burst));
    }
}