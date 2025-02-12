#include <stdio.h>
#include <stdlib.h>             // memory alloc
#include <unistd.h>             // fork(), execv()
#include <signal.h>             // signal handling
#include <stdbool.h>            //
#include <sys/wait.h>           // wait system call header
#include <string.h>             //
#include <errno.h>              // error handling

//parent process, spawns child processes representing gates, based on the input
//Signal Handlers: sigusr1, sigterm, sigchild
//parses command line arguments, sets up signal handlers, initializes gate states

pid_t *pid_child =  NULL;       //pointer to store child process IDs
pid_t pid_parent;
int children = 0;               //number of child processes
int status;                     //stores status info of child processes
char *gate_state;               //pointer to store states(t/f)
bool terminate = false;         //boolean to terminate child processes

bool valid_string(char *str){   //checks if the given string contains only the chars t and f
 for(size_t i=0; i<strlen(str); i++){ // iterate through string
  if(!(str[i] =='t' || str[i] == 'f')) return false;
 }
 return true;
}

void sigusr1_handler(int sig_num){ //send the SIGUSR1 signal to all child processes
 for(int i=0; i<children; i++){
  if (kill(pid_child[i], SIGUSR1) == -1) {perror("kill"); exit(1);}
 }
}

void sigterm_handler(int sig_num){ //handles the SIGTERM signal by sending the termination signal to all child processes
 terminate=true;
 for(int i=0; i<children; i++){
  printf("[PARENT/PID=%d] Waiting for %d children to exit\n", pid_parent, children-i);
  if (kill(pid_child[i], SIGTERM) == -1){ perror("kill"); exit(1);}   //terminate child process i gracefully
  waitpid(pid_child[i], NULL, 0); // wait for termination of child process i , no status info needed, default behavior
  printf("[PARENT/PID=%d] [Child/PID=%d] terminated with exit status code%d!\n", pid_parent, pid_child[i], status);
  }

 printf("[PARENT/PID=%d] all children exited-terminating\n", pid_parent);
 exit(0);
}

void sigchild_handler(int sig_num){          //signal is sent to parent process when a child process terminates or is stopped
 if(!terminate){                             //check if in process of termination, don't restore children
  int child_status;                          //stores status of child process
  pid_t childpid;                            //store process id of child
                                             // search for the index of the stopped/terminated process
  
  while ((childpid= waitpid(-1, &child_status, WUNTRACED))>0){     //call waitpid repeatedly, wait for termination of          processes,WUNTRACED:function should return if child has stopped
   int child_index = -1; 
   for(int i=0; i<children; i++){
    if (childpid == pid_child[i]){  // check if childpid matches the process id stored at index i, if it matches assign i to  child_index
     child_index=i;
     break;
    }
   }

   if(child_index != -1){ 
    if(WIFEXITED(child_status)) {                                                          //checks if child exited normally
     printf("[PARENT/PID=%d] Child %d/PID=%d exited\n", pid_parent, child_index, childpid);
     char id_string[50];                                                                   //index of child process
     char state_string[50];                                                                //state of gate corresponding to child process
     sprintf(id_string, "%d", child_index);
     sprintf(state_string, "%c", (gate_state[child_index] == 't') ? 't' :'f');
     char * arguments[] = {"./child", id_string, state_string, NULL};                      //contains arguments to be passed to the execv function. (EXECUTABLE PATH, INDEX OF CHILD PROCESS, STATE OF GATE,  NULL TO TERMINATE ARGUMENT LIST)

     pid_t new_pid = fork(); //new child process
     if(new_pid == -1) {     //error occured
      perror("fork");
      exit(1);
     }
     else if (new_pid == 0 ) {      //replace child process with new instance
      execv("./child", arguments);  //system call to execute new program (executable, arguments to be passed)
      perror("execv");
      exit(1);
     }
     else {                   //update pid_child array
      pid_child[child_index] = new_pid;
      printf("[PARENT/PID=%d] Created new child for gate %d (PID %d) and initial state '%c'\n", pid_parent, child_index, new_pid, state_string[0]);
     }
    }
    else if( WIFSTOPPED(child_status)) {                                     //if child stopped continue
     printf("PARENT/PID=%d] [Child/PID=%d] stopped \n",pid_parent, childpid);
     if(kill(childpid,SIGCONT) == -1) {perror("kill"); exit(1);}
     printf("PARENT/PID=%d] [Child/PID=%d] resumed \n",pid_parent, childpid);
    } 
   }
  }

 if(childpid == -1 && errno != ECHILD && errno != EINTR) perror("waitpid");  //check if error occured, not due to absence of child processes(ECHILD) or interruption by signal other that SIGCHILD('EINTR')
 }
}

int main(int argc, char *argv[]) {
 if( argc!=2 || !valid_string(argv[1])){ //command line arguments are not 2 and the string is valid
  printf("Usage: %s <string from  (f,t)>\n", argv[0]);
  return 1;
  }

 gate_state = argv[1];
 pid_parent=getpid();
 children= strlen(gate_state);
 
 //function calls
 struct sigaction action;                 //when the SIGUSR1 is received invoke the sigusr1_header function

 action.sa_handler = sigusr1_handler;
 sigemptyset(&action.sa_mask);            //clear signal mask->no signals are blocked
                                          //int sigaction( int signum, const struct sigaction *act, struct sigaction *oldact)
 action.sa_flags=0;                       //no special flags are set
 if(sigaction(SIGUSR1, &action, NULL) == -1) {perror("sigaction"); return 1;}

 action.sa_handler = sigterm_handler;
 if(sigaction(SIGTERM, &action, NULL) == -1) {perror("sigaction"); return 1;}

 action.sa_handler = sigchild_handler;
 if (sigaction(SIGCHLD, &action, NULL) == -1){perror("sigaction"); return 1;}
 
 pid_child = malloc((children+1)* sizeof(pid_t));//dynamic allocation for pid_child array
 if (pid_child==NULL) {perror("realloc");return 1;}

 
 char id_string[50];
 char state_string[50];
 for (int i=0; i<children; i++){//child process creation
  pid_t pid = fork();
  if (pid == -1) { perror("fork"); return 1;}
  else if (pid == 0) {
   sprintf(id_string,"%u", i);
   sprintf(state_string, "%c" , argv[1][i]);
   char *arguments[] = {"./child", id_string, state_string, NULL};//ARRAY OF STRINGS
   execv("./child", arguments);
   perror("execv");
   return 1;
  }
  else {
   printf("[PARENT/PID=%d] created child %u (PID=%d) and initial state '%c'\n", pid_parent, i, pid, argv[1][i]);
   pid_child[i]=pid; //store process id
   }
  }
 

 while(true) sleep(1); //ensure program does not terminate 

 return 0;
}
