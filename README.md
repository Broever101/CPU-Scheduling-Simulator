# BROEVER CERTIFIED SUPER PLUS ULTRA FANTASTIC USELESS LAME CPU SCHEDULING SIMULATOR

This is a work in progress. Or not.

The idea is to simulate the scheduling of processes in a uniprocessor environment using all the outdated C libraries for linux (it was a requirement of the assignment). 

To use the program, build the state processes as follows:
```g++ -o new_state utilities.cpp new_state.cpp
g++ -o exit_state utilities.cpp exit_state.cpp
g++ -o running_state utilities.cpp running_state.cpp
g++ -pthread -o blocked_state utilities.cpp blocked_state.cpp
g++ -pthread -o ready_state utilities.cpp ready_state.cpp
g++ -o 5_states 5_state_model_simulator.cpp
```

To run the program:
```./5_states sample_file.txt
```

The sample_file.txt will contain information about processes.
*Sample format of FCFS/SJF/STRF
FCFS
Proc1
Arrival time
Burst time
Proc2
Arrival time
Burst time*

*Sample format of RR
RR
Quantum
Proc1
Arrival time
Burst time
Proc2
Arrival time
Burst time*

