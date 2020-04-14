#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include <sstream>
#include <algorithm>
#include "Process.h"
#include <string.h>
#include <memory>

typedef std::vector<std::shared_ptr<Process>> p_vector;

void writeToPipe(int, std::string&);
std::string readFromPipe(int);
void incrementProcs(p_vector&);
std::string createPacket(const std::shared_ptr<Process>&);
void createProcs(std::string data, p_vector&);
void scheduleProc(const int&, p_vector&, p_vector&);

int main(int argc, char* argv[]){
    int new_ready = open("new2ready", O_RDONLY, O_NONBLOCK);
    if (new_ready < 0) std::cout<<"Could not open new2ready in ready.\n";
    int ready_running = open("ready2running", O_WRONLY);
    if (ready_running < 0) std::cout<<"Could not open ready2running in ready.\n";
    int running_ready = open("running2ready", O_RDONLY, O_NONBLOCK);
    if (running_ready < 0) std::cout<<"Could not open running2ready in ready.\n";
    int blocked_ready = open("block2ready", O_RDONLY, O_NONBLOCK);
    if (blocked_ready < 0) std::cout<<"Could not open block2ready in ready.\n";
    
    std::string scheduling_algo = readFromPipe(new_ready);
    
    while (scheduling_algo == "empty")
        scheduling_algo = readFromPipe(new_ready);
    
    p_vector procs;
    p_vector scheduled;
    std::string packet;

    std::string data = readFromPipe(new_ready);
    while (data == "empty")
        data = readFromPipe(new_ready);
    createProcs(data, procs);
    for (auto i : procs)
        std::cout<<"READY: "<<i->proc_name<<"arrived from NEW.\n";
    /* SORT THE PROC VECTOR IN DESCENDING ORDER HERE*/
    scheduleProc(running_ready, procs, scheduled);

    do {     
        do {
            data = readFromPipe(new_ready);
            if (data != "empty" || data != "closed"){
                createProcs(data, procs);
                std::cout<<"READY: "<<procs.rbegin()->get()->proc_name<<"arrived from NEW.\n";
            }
            data = readFromPipe(blocked_ready);
            if (data != "empty" || data != "closed"){
                createProcs(data, procs);
                scheduled.erase(std::find(scheduled.begin(),
                scheduled.end(), *procs.rbegin()));
                std::cout<<"READY: "<<procs.rbegin()->get()->proc_name<<"returned from BLOCK.\n";
                if (scheduled.empty()){
                    /*SORT PROCS*/
                    scheduleProc(running_ready, procs, scheduled); 
                }
            }
            sleep(1);
            incrementProcs(procs);
            data = readFromPipe(running_ready);
        } while (data == "empty");
        createProcs(data, procs);
        scheduled.erase(std::find(scheduled.begin(),scheduled.end(), *procs.rbegin()));
        
        if ((*procs.rbegin())->remaining_burst == 0)
            procs.pop_back();

        if (procs.empty()){
            std::cout<<"READY: Queue empty.\n";
            break;
        }

        /* SORT PROCS*/
        scheduleProc(running_ready, procs, scheduled); 
    } while (!scheduled.empty());
    
    close(new_ready);
    close(ready_running);
    close(running_ready);
    close(blocked_ready);
    exit(0);
}

void scheduleProc(const int& running_ready, p_vector& procs, p_vector& scheduled){
    std::string packet = createPacket(*procs.rbegin());
    writeToPipe(running_ready, packet);
    std::cout<<"READY: "<<(*procs.rbegin())->proc_name<<"scheduled.\n";
    scheduled.push_back(*procs.rbegin());
    procs.pop_back(); 
}

void incrementProcs(p_vector& procs){
    for (auto i: procs){
        i->turnaround++;
        i->waiting++;
    }
}

std::string createPacket(const std::shared_ptr<Process>& proc){
    return std::string(proc->proc_name + "\n" +
                std::to_string(proc->arrival) + "\n" +
                std::to_string(proc->burst) + "\n" +
                std::to_string(proc->remaining_burst) + "\n" +
                std::to_string(proc->turnaround) + "\n" +
                std::to_string(proc->waiting));
}

void printVector(const p_vector& procs){
    for (auto i: procs){
        std::cout<<i->proc_name<<std::endl;
        std::cout<<i->arrival<<std::endl;
        std::cout<<i->burst<<std::endl;
    }
}

void createProcs(std::string data, p_vector& procs){
    std::istringstream stream(std::move(data));
    std::string proc; 
    size_t arr, burst, rem_burst, turn, wait;
    while(stream>>proc){
        stream>>arr>>burst>>rem_burst>>turn>>wait;
        procs.push_back(std::make_shared<Process>(proc, arr, burst, rem_burst, turn, wait));
    }
}

void writeToPipe(int pipe_fd, std::string& message){
    write(pipe_fd, message.c_str(), message.size()+1);
}

std::string readFromPipe(int pipe_fd){
    char msg[256];
    size_t bytes = read(pipe_fd, msg, sizeof(msg));
    if (bytes == 0) return "closed";
    if (bytes < 0) return "empty";
    msg[bytes] = '\0';
    return msg;
}