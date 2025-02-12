
#include <sys/wait.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])                                          //argc:represents number of command line arguments passed 
{                                                                         //argv[]: represents an array of strings
                                                                          //containing command line arguments

 struct stat file;                                                        //stores file info
  
  if(argc != 2){                                                          //arguments are not more than  2 --> error
   printf("Usage: .a/.out filename\n");
   return 1;
  }
  if(strcmp(argv[1],"--help") == 0){                                     //if given flag --help --> successful execution
   printf("Usage: .a/.out filename\n");
   return 0;
    }
  if(stat(argv[1], &file) == 0){                                    //if  file exists already --> error
   printf("Error: %s already exists\n",argv[1]);
   return 1;
    }
  

 int fd = open(argv[1], O_CREAT | O_APPEND | O_WRONLY | O_RDONLY , 0644); //open system call, read and write, file permission 

 if (fd == -1) {                                                          // check if open system call failed
  perror("open");
  return 1;
 }

 int status;
 pid_t child;

 child = fork();                                                          //child process using fork system, stores return value 
                                                                          //in child
                                                                          //ERROR CODE

 if(child<0){                                                             //check if fork failed
  printf("Error in generating  child procedure");
  return 1;
  }
                                                                          //CHILD CODE

 if(child==0){                                                            //retrieve child&parent procs
  pid_t childid = getpid();
  pid_t parentid = getppid();
  char bufchild[50];
  sprintf(bufchild,"[CHILD] getpid()= %d,getppid()=%d\n", childid, parentid);

  if(write(fd, bufchild, strlen(bufchild)) < strlen(bufchild)) {                          //write message string to file descriptor fd
   perror("write");                                                       //if write op fails --> error
   return 1;
  }
  exit(0);                                                                //exit child process

  }
                                                                          //PARENT CODE
else {
  wait(&status);                                                          //in the parent process wait for child process to finish
  pid_t childidf = getpid();                                              //retrieve child&parent procs
  pid_t parentidf = getppid();
  char bufparent[50];
  sprintf(bufparent,"[PARENT] getpid()= %d,getppid()=%d\n", childidf, parentidf);

  if(write(fd, bufparent, strlen(bufparent)) < strlen(bufparent)) {
   perror("write");
   return 1;
  }

  close(fd);                                                              //close fd
  if(close<0)
  {
   perror("close error");
   return 1;
  }
  exit(0);                                                                // exit parent process
 }
  
 return 0;
}
