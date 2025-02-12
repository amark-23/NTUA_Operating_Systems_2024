#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/types.h>
#include <time.h>

#define ALARM_TIME 15

//CHILD PROGRAM
//REPRESENTS INDIVIDUAL GATE
//REACTS TO SIGNALS FROM PARENT
//REPORTS STATE CHANGES

int id;
pid_t pid, ppid;                                                //child process ID, process ID, parent process ID
bool gate_state;
time_t start_time;

void sigusr1_handler(int sig_num){                              //sigusr1 signal, prints current state
 printf("[ID=%d/PID=%d/TIME=%lds] the gates are %s.\n", id, getpid(), time(NULL)-start_time, gate_state? "open":"closed");
 }

void sigusr2_handler(int sig_num){                              //sigusr2 signal, toggles gate state, sends sigusr1 signal to parent process
 gate_state=!gate_state;
 if(kill(pid, SIGUSR1) == -1){
  perror("kill");
  exit (1);
  }
}

void timer_handler(int sig_num){   //sigalrm signal
 alarm(ALARM_TIME);
 if(kill(pid, SIGUSR1) == -1){
  perror("kill");
  exit (1);
 }
}

void sigterm_handler(int signum) { //send sigchld to parent 
 if(kill(ppid, SIGCHLD) == -1){
  perror("kill");
  exit (1);
 }
 exit(0);
}

int main(int argc, char *argv[]){
 if (argc != 3) { 
  fprintf(stderr, "Usage: %s <ID> <state>\n", argv[0]);
  return 1;
 }

 id=atoi(argv[1]); //convert string to  int
 pid=getpid();
 ppid=getppid();

 if(argv[2][0] =='t') gate_state=true;
 else gate_state=false;

 struct sigaction action;
 action.sa_handler = sigusr1_handler;
 sigemptyset(&action.sa_mask);
 action.sa_flags=0;
 
 //signal handlers
 if (sigaction(SIGUSR1,&action, NULL)==-1){ perror("sigaction");return 1;}

 action.sa_handler=sigusr2_handler;
 if (sigaction(SIGUSR2,&action, NULL)==-1){ perror("sigaction");return 1;}

 action.sa_handler=timer_handler;
 if (sigaction(SIGALRM,&action, NULL)==-1){ perror("sigaction");return 1;}

 action.sa_handler=sigterm_handler;
 if (sigaction(SIGTERM,&action, NULL)==-1){ perror("sigaction");return 1;}
 
 
 start_time = time(NULL);
 if(kill(pid, SIGALRM)==-1){
  perror("kill");
  exit(1);
  }
 while(1) sleep(1);//ensure program does not terminate

 return 0;
}







