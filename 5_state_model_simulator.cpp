#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>


int createNewFifo( const char *fifoName, int permissions ){
    struct stat stats;
    if ( stat( fifoName, &stats ) < 0 ){
        if ( errno != ENOENT ){          
            perror( "Error: Stat Failed." );    
            return( -1 );
        }
    }
    else{                                
        if ( unlink( fifoName ) < 0 ){   
            perror( "Error: Unlink Failed" );  
            return( -1 );
        }
    }
    if ( mkfifo( fifoName, permissions ) < 0 ){ 
        perror("Error: Cannot create pipe.");
        return( -1 );
    }
    return( 0 );
}

int main(){
    char* args[] = {NULL};
    pid_t new_state = fork();
    if (new_state == 0){
        if (createNewFifo("new2ready", 0666) < 0) exit(1);
        else std::cout<<"new2ready pipe created.\n";
        execvp("./new_state", args);
        std::cout<<"NEW STATE EXEC FAILED\n";
    }
    else if (new_state > 0){
        pid_t ready_state = fork();
        if (ready_state == 0){
            execvp("./ready_state", args);
            std::cout<<"READY STATE EXEC FAILED\n";
        }
        else if (ready_state > 0){
            pid_t running_state = fork();
            if (running_state == 0){
                execvp("./running_state", args);
                std::cout<<"RUNNING STATE EXEC FAILED\n";
            }
            else if (running_state > 0){
                pid_t blocked_state = fork();
                if (blocked_state == 0){
                    execvp("./blocked_state", args);
                    std::cout<<"BLOCKED STATE EXEC FAILED\n";
                }
                else if (blocked_state > 0){
                    pid_t exit_state = fork();
                    if (exit_state == 0){
                        execvp("./exit_state", args);
                        std::cout<<"EXIT STATE EXEC FAILED\n";
                    }
                    else if (exit_state > 0){
                        std::cout<<"ALL STATE PROCESSES CREATED\n";
                    }
                }
            }
        }
        
    }

}
