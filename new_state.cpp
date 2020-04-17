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
#include <memory>
#include <algorithm>
#include "utilities.h"

int main(int argc, char *argv[])
{
    int new_ready = open("new2ready", O_WRONLY | O_APPEND);
    if (new_ready < 0)
        std::cout << "Could not open new2ready in new.\n";

    p_vector procs;
    std::string scheduling_algo;
    std::string time_quantum = "-1";

    std::string file_path = argv[1];
   
    int proc_file = open(file_path.c_str(), O_RDONLY);
    if (proc_file < 0)
    {
        std::cout << file_path << std::endl;
        perror("Error ");
    }

    std::string data = utils::readFromFile(proc_file);
    close(proc_file);

    utils::createProcs(std::move(data), scheduling_algo, time_quantum, procs);
    utils::writeToPipe(new_ready, scheduling_algo);
    sleep(1);
    utils::writeToPipe(new_ready, time_quantum);
    sleep(1);

    std::stable_sort(procs.begin(), procs.end(),
                     [](auto proc1, auto proc2) {
                         if (proc1->arrival < proc2->arrival)
                             return true;
                         else if (proc1->proc_name < proc2->proc_name)
                             return true;
                     });

    size_t time = 0;
    for (auto i : procs)
    {
        while (time != i->arrival)
        {
            sleep(1);
            ++time;
        }
        std::string packet = utils::createPacket(i);
        //std::cout<<packet<<std::endl;
        utils::writeToPipe(new_ready, packet);
        std::cout << "NEW: " << i->proc_name << " admitted to READY.\n";
        sleep(1);
    }
    close(new_ready);
    exit(0);
}
