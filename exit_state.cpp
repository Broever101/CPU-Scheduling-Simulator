#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
//#include "Process.h"
#include <memory>
#include <sstream>
//#include <fcntl.h>
#include "utilities.h"

//typedef std::shared_ptr<Process> process;
//oid createProc(std::string, process&);
//std::string createPacket(const process&);
//std::string readFromPipe(int);


int main(int argc, char *argv[])
{
    int running_exit = open("running2exit", O_RDONLY);
    if (running_exit < 0)
        std::cout << "Could not open running2exit in exit.\n";

    process proc(nullptr);
    std::string data = utils::readFromPipe(running_exit);
    while (data != "closed")
    {
        utils::createProc(data, proc);
        std::cout << "EXIT: " << proc->proc_name << " exited.\n";
        data = utils::readFromPipe(running_exit);
    }
    close(running_exit);
    exit(0);
}
