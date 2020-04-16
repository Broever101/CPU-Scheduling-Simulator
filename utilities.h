#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <sstream>
#include <fcntl.h>
#include "Process.h"

typedef std::shared_ptr<Process> process;
typedef std::vector<std::shared_ptr<Process>> p_vector; 

namespace utils{
    extern void writeToPipe(int, std::string&);
    extern std::string readFromPipe(int);
    extern std::string createPacket(const process&);
    extern void createProcs(std::string, p_vector&);
    extern void printVector(const p_vector&);
    extern void setNonBlock(const int&);
    extern std::string readFromFile(const int &);
    extern void createProcs(std::string, std::string &, std::string &, p_vector &);
    extern void createProc(std::string, process&);
}