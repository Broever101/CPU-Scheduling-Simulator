#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>
#include <sstream>
#include <algorithm>
#include <string.h>
#include <memory>
#include <map>
#include "utilities.h"

typedef bool (*scheduler)(const process &, const process &);

void incrementProcs(p_vector &);
void scheduleProc(const int &, p_vector &, p_vector &);
void removeFromScheduled(p_vector &, Process &);
bool FCFS(const process &, const process &);
bool SJF(const process &, const process &);
bool SRTF(const process &, const process &);
bool RR(const process &, const process &);

int main(int argc, char *argv[])
{
    int new_ready = open("new2ready", O_RDONLY);
    if (new_ready < 0)
        std::cout << "Could not open new2ready in ready.\n";
    int ready_running = open("ready2running", O_WRONLY);
    if (ready_running < 0)
        std::cout << "Could not open ready2running in ready.\n";
    int running_ready = open("running2ready", O_RDONLY);
    if (running_ready < 0)
        std::cout << "Could not open running2ready in ready.\n";
    int blocked_ready = open("block2ready", O_RDONLY);
    if (blocked_ready < 0)
        std::cout << "Could not open block2ready in ready.\n";

    p_vector procs;
    p_vector scheduled;
    std::string packet = "-1";

    std::string scheduling_algo = utils::readFromPipe(new_ready);

    //std::cout<<scheduling_algo<<std::endl;
    packet = utils::readFromPipe(new_ready);

    /* SEND TIME QUANTUM*/
    std::cout<<"SENDING QUANTUM: "<<packet<<std::endl;
    utils::writeToPipe(ready_running, packet);

    std::map<std::string, scheduler> getAlgorithm = {
        {"FCFS", &FCFS}, {"RR", &RR}, {"SJF", &SJF}, {"SRTF", &SRTF}};

    scheduler comparator = getAlgorithm[scheduling_algo];

    std::string data = utils::readFromPipe(new_ready);

    utils::createProcs(data, procs);
    std::stable_sort(procs.begin(), procs.end(), comparator);

    for (auto i : procs)
        std::cout << "READY: " << i->proc_name << " arrived from NEW.\n";

    scheduleProc(ready_running, procs, scheduled);
    utils::setNonBlock(blocked_ready);
    utils::setNonBlock(running_ready);
    do
    {
        do
        {
            data = utils::readFromPipe(new_ready);
            if (data != "empty" && data != "closed")
            {
                utils::createProcs(data, procs);
                std::cout << "READY: " << procs.rbegin()->get()->proc_name << " arrived from NEW.\n";
            }
            data = utils::readFromPipe(blocked_ready);
            if (data != "empty" && data != "closed")
            {
                utils::createProcs(data, procs);
                removeFromScheduled(scheduled, *(*procs.rbegin()));
                std::cout << "READY: " << procs.rbegin()->get()->proc_name << " returned from BLOCK.\n";
                if (scheduled.empty())
                {
                    std::stable_sort(procs.begin(), procs.end(), comparator);
                    scheduleProc(ready_running, procs, scheduled);
                }
            }
            sleep(1);
            incrementProcs(procs);
            data = utils::readFromPipe(running_ready);
        } while (data == "empty");

        if (data != "NEXT")
        {
            utils::createProcs(data, procs);
            removeFromScheduled(scheduled, *(*procs.rbegin()));
        }

        if ((*procs.rbegin())->remaining_burst == 0)
            procs.pop_back();

        if (procs.empty())
        {
            std::cout << "READY: QUEUE EMPTY.\n";
            std::cout << "REMAINING SCHEDULED: \n";
            for (auto i : scheduled)
            {
                std::cout << i->proc_name << std::endl;
            }
        }
        else
        {
            std::stable_sort(procs.begin(), procs.end(), comparator);
            scheduleProc(ready_running, procs, scheduled);
        }
    } while (!(scheduled.empty() && procs.empty()));
    std::cout << "READY : ALL PROCESSES SCHEDULED.\n";
    close(new_ready);
    close(ready_running);
    close(running_ready);
    close(blocked_ready);
    exit(0);
}

void removeFromScheduled(p_vector &scheduled, Process &proc)
{
    for (auto i = scheduled.begin(); i < scheduled.end(); i++)
    {
        if (*(i->get()) == proc)
        {
            scheduled.erase(i);
        }
    }
}

void scheduleProc(const int &ready_running, p_vector &procs, p_vector &scheduled)
{
    std::string packet = utils::createPacket(*procs.rbegin());
    utils::writeToPipe(ready_running, packet);
    std::cout << "READY: " << (*procs.rbegin())->proc_name << " scheduled.\n";
    scheduled.push_back(*procs.rbegin());
    procs.pop_back();
}

void incrementProcs(p_vector &procs)
{
    for (auto i : procs)
    {
        i->turnaround++;
        i->waiting++;
    }
}

bool FCFS(const process &proc1, const process &proc2)
{
    if (proc1->arrival > proc2->arrival)
        return true;
    else if (proc1->proc_name > proc2->proc_name)
        return true;
}

bool SRTF(const process &proc1, const process &proc2)
{
    if (proc1->remaining_burst > proc2->remaining_burst)
        return true;
}

bool SJF(const process &proc1, const process &proc2)
{
    if (proc1->burst > proc2->burst)
        return true;
}

bool RR(const process &proc1, const process &proc2)
{
    if (proc1->priority > proc2->priority)
        return true;
}
