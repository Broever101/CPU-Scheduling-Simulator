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
#include <pthread.h>
#include "utilities.h"

typedef bool (*scheduler)(const process &, const process &);

pthread_t new_thread, block_thread, terminate_thread;

void incrementProcs(p_vector &);
void scheduleProc(const int &, p_vector &, p_vector &);
void removeFromScheduled(p_vector &, Process &);
void* readFromBlock(void*);
void* readFromNew(void*);
void* terminate(void*);
bool FCFS(const process &, const process &);
bool SJF(const process &, const process &);
bool SRTF(const process &, const process &);
bool RR(const process &, const process &);

struct ThreadDumps{
    int& fd;
    std::string& data;
    p_vector& procs;
    p_vector& scheduled;
    ThreadDumps(int& fd, std::string& data, p_vector& procs, p_vector& scheduled):
    fd(fd), data(data), procs(procs), scheduled(scheduled) {}
};

int main(int argc, char *argv[])
{
    const int new_ready = open("new2ready", O_RDONLY);
    if (new_ready < 0)
        std::cout << "Could not open new2ready in ready.\n";
    const int ready_running = open("ready2running", O_WRONLY);
    if (ready_running < 0)
        std::cout << "Could not open ready2running in ready.\n";
    const int running_ready = open("running2ready", O_RDONLY);
    if (running_ready < 0)
        std::cout << "Could not open running2ready in ready.\n";
    const int blocked_ready = open("block2ready", O_RDONLY);
    if (blocked_ready < 0)
        std::cout << "Could not open block2ready in ready.\n";

    p_vector procs;
    p_vector scheduled;
    std::string data = "-1";
    ThreadDumps dumpForNew(const_cast<int&>(new_ready), data, procs, scheduled);
    ThreadDumps dumpForBlock(const_cast<int&>(blocked_ready), data, procs, scheduled);

    std::string scheduling_algo = utils::readFromPipe(new_ready);

    data = utils::readFromPipe(new_ready);

    /* SEND TIME QUANTUM*/
    std::cout<<"SENDING QUANTUM: "<<data<<std::endl;
    utils::writeToPipe(ready_running, data);

    std::map<std::string, scheduler> getAlgorithm = {
        {"FCFS", &FCFS}, {"RR", &RR}, {"SJF", &SJF}, {"SRTF", &SRTF}};

    /*SELECT SCHEDULING ALGORITHM*/
    scheduler comparator = getAlgorithm[scheduling_algo];

    data = utils::readFromPipe(new_ready);

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
            pthread_create(&new_thread, NULL, &readFromNew, &dumpForNew);
            pthread_create(&block_thread, NULL, &readFromBlock, &dumpForBlock);
            pthread_join(block_thread, NULL);
            if (scheduled.empty())
            {
                std::stable_sort(procs.begin(), procs.end(), comparator);
                scheduleProc(ready_running, procs, scheduled);
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
        
        if (!procs.empty() && (*procs.rbegin())->remaining_burst == 0)
                procs.pop_back();

        if (procs.empty())
        {
            std::cout << "READY: QUEUE EMPTY.\n";
            std::cout << "READY: REMAINING PROCESSES IN SCHEDULED: \n";
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
        pthread_create(&terminate_thread, NULL, &terminate, &dumpForNew);
        pthread_join(terminate_thread, NULL);
        std::cout << "READY: PROCESSES IN SCHEDULED QUEUE: \n";
            for (auto i : scheduled)
            {
                std::cout << i->proc_name << std::endl;
            }
    } while (!(scheduled.empty() && procs.empty()));
    
    std::cout << "READY : ALL PROCESSES SCHEDULED.\n";
    close(new_ready);
    close(ready_running);
    close(running_ready);
    close(blocked_ready);
    pthread_exit(NULL);
}

void* readFromNew(void* dump){
    ThreadDumps* dumpForNew = (struct ThreadDumps*) dump;
    dumpForNew->data = utils::readFromPipe(dumpForNew->fd);
    if (dumpForNew->data != "empty" && dumpForNew->data != "closed")
    {
        utils::createProcs(dumpForNew->data, dumpForNew->procs);
        std::cout << "READY: " << dumpForNew->procs.rbegin()->get()->proc_name << " arrived from NEW.\n";
    }
    pthread_exit(NULL);
}

void *terminate(void *dump)
{
    ThreadDumps *dumpForNew = (struct ThreadDumps *)dump;
    if (dumpForNew->procs.empty() && dumpForNew->scheduled.empty())
    {
        std::cout << "READY: Waiting for NEW processes.\n";
        fcntl(dumpForNew->fd, F_SETFL, fcntl(dumpForNew->fd, F_GETFL) & ~O_NONBLOCK);
        dumpForNew->data = utils::readFromPipe(dumpForNew->fd);
        if (dumpForNew->data == "closed")
        {
            std::cout << "READY: No more incoming processes.\n";
            pthread_exit(NULL);
        }
        else
        {
            utils::createProcs(dumpForNew->data, dumpForNew->procs);
            std::cout << "READY: " << dumpForNew->procs.rbegin()->get()->proc_name
                      << " arrived from NEW.\n";
            utils::setNonBlock(dumpForNew->fd);
            pthread_exit(NULL);
        }
    }
    pthread_exit(NULL);
}


void* readFromBlock(void* dump){
    pthread_join(new_thread, NULL);
    ThreadDumps* dumpForBlock = (struct ThreadDumps*) dump;
    dumpForBlock->data = utils::readFromPipe(dumpForBlock->fd);
    if (dumpForBlock->data != "empty" && dumpForBlock->data != "closed")
    {
        utils::createProcs(dumpForBlock->data, dumpForBlock->procs);
        removeFromScheduled(dumpForBlock->scheduled, *(*dumpForBlock->procs.rbegin()));
        std::cout << "READY: " << dumpForBlock->procs.rbegin()->get()->proc_name << " returned from BLOCK.\n";
        
    }
    pthread_exit(NULL);
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
