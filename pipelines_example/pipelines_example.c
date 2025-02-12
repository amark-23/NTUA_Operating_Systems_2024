#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <strings.h>
#include <string.h>
#include<stdbool.h>
#include <errno.h>
#include<ctype.h>
#include <sys/select.h>
#include <time.h>
#include <poll.h>
#define BUFFER_SIZE 64

//in this program, father-process F creates n children-processes
//F process sends tasks("jobs") through pipelines 
//
//Pipelines --> Allows processes to communicate 
//Two ends: Read-End (Receiver) & Write-End (Sender)

//This function is used to check if a process returns negative value(error)
//print error through perror to terminal

void isnegative(int returnval, const char *message) 
{
  if (returnval < 0) {
    perror(message);
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char **argv)
{
  bool round_robin_scheduling = false; 
  //round-robin scheduling: Round-robin scheduling is a method of scheduling tasks 
  //or processes in a computer system where each task or process receives an equal 
  //share of the CPU's time in a cyclic manner.
  //In the context of the program it describes a cyclic scheduling 
  bool random_scheduling = false;
  //program distributes job to children randomly
  int number_of_processes ;
  int *p_id_array;
  //array to store process ids of children 
  char buffer[BUFFER_SIZE];
  //array of characters used for reading input from user

  //Usage case: 1 argument plus program name
  if (argc == 2)
  //User provided only one argument besides the program name
  //This means the default(round robin) scheduling method is used
  //We check if user provided a positive integer
  {
    for(int i=0; i<strlen(argv[1]); i++)
    {
        if (!isdigit(argv[1][i]))
        {
            printf("Usage: ask3 <nChildren> [--random] [--round-robin]\n");
            exit(1);
        }
    }
    round_robin_scheduling = true;
  }
  
   //Usage case: 2 arguments plus program name
   else if (argc == 3)
    //We if check user provided a positive integer in the second argument
    //We also check if the user has provided the scheduling mode correctly
    {
        for(int i=0; i<strlen(argv[1]); i++)
        {
          if (!isdigit(argv[1][i]))
          {
            printf("Usage: ask3 <nChildren> [--random] [--round-robin]\n");
            exit(1);
          }
        }
        //strcmp(str1,str2): used to compare two strings in lexicographical order
        //str1==str2: returns 0
        //str1<str2: returns negative value
        //str1>str2: returns positive value
        if(strcmp(argv[2],"--round-robin") == 0){
            round_robin_scheduling = true;
        }
        else if(strcmp(argv[2],"--random") == 0){
            random_scheduling = true;
        }
        else{
            printf("Usage: ask3 <nChildren> [--random] [--round-robin]\n");
            exit(1);
        }
    }
    else
    {
      printf("Usage: ask3 <nChildren> [--random] [--round-robin]\n");
      exit(1);
    }
   
   //atoi(char c): string to integer
   number_of_processes = atoi(argv[1]); 

   if (number_of_processes <= 0) 
    {
        printf("Invalid number of children: %s\n", argv[1]);
        exit(1);
    }
   
   //sizeof(datatype):This operator returns the size of a data type in bytes.
   //malloc:(byte number): This function allocates memory dynamically from the heap.
   p_id_array = (int *)malloc(number_of_processes * sizeof(int)); 
   if (p_id_array == NULL) 
    {
      printf("Memory allocation failed.\n");
      return 1;
    }
   
   
   //pipe_fd_child2parent and pipe_fd_parent2child are arrays of arrays used to store file descriptors for pipes.
   //A file descriptor is a unique integer value used to identify and access an open file or I/O 
   //resource in a computer operating system. 
   //In this program, file descriptors are used by the pipe() system call to set up pipelines.
   //The pipes are bidirectional and we can choose the direction we use.
   
   int **pipe_fd_parent2child;
   //This block of code declares a pointer to a pointer to an integer, which will 
   //be used to store the file descriptors for the pipes.
   //It also allocates memory for an array of pointers to integers, where each pointer points to an 
   //array of two integers representing the file descriptors for a pipe (one for reading and one for writing).
   
   //Each element of pipe_fd_parent2child and pipe_fd_child2parent represents 
   //a pipe between the parent process and a specific child process. 
   
   //The first element of each sub-array is the file descriptor for writing to the child process (parent-to-child communication),
   //and the second element is the file descriptor for reading from the child process (child-to-parent communication).
   
   pipe_fd_parent2child = (int **)malloc(number_of_processes * sizeof(int *));  
   for (int i = 0; i < number_of_processes; i++) 
    {
        pipe_fd_parent2child[i] = (int *)malloc(2 * sizeof(int));
        //memory is allocated for an array of two integers to store the file descriptors of the pipe.
        //One integer will represent the read end of the pipe, and the other will represent the write end.  
        if (pipe_fd_parent2child[i] == NULL) 
        { 
            printf("Memory allocation failed");
            exit(1);
        }
    }

    int **pipe_fd_child2parent;
   //The first element of each sub-array is the file descriptor for writing to the parent process (child-to-parent communication),
   //and the second element is the file descriptor for reading from the parent process (parent-to-child communication).
    pipe_fd_child2parent = (int **)malloc(number_of_processes * sizeof(int *)); 
    for (int i = 0; i < number_of_processes; i++) 
    {
        pipe_fd_child2parent[i] = (int *)malloc(2 * sizeof(int));
        if (pipe_fd_child2parent[i] == NULL) 
        {   
            printf("Memory allocation failed");
            exit(1);
        }
    }
  
  
  
  //int pipe(int fd[2]) system call:
  //
  //creates a pipeline between two processes
  //
  //Parameters:
  //pipefd: An array of two integers that will store the file descriptors for the pipe. 
  //pipefd[0] refers to the read end of the pipe.
  //pipefd[1] refers to the write end of the pipe.
  //
  //Return Value:
  //On success returns 0.
  //On failure returns -1.
  //
  //After a successful call to pipe(), pipefd[0] and pipefd[1] will contain the file descriptors
  //for the read and write ends of the pipe, respectively.
  //Data written to the write end of the pipe (pipefd[1]) can be read from the read end of the pipe (pipefd[0]).
  
  
  
  //i indicates child process
  //pipe_fd_parent2child[i][0] will hold the file descriptor for reading from the pipe (from parent to child) 
  //pipe_fd_parent2child[i][1] will hold the file descriptor for writing to the pipe (from child to parent).
  for (int i = 0; i < number_of_processes; i++)
    {
        if (pipe(pipe_fd_parent2child[i]) == -1) {           
            perror("pipe");
            exit(1);
        }
    }
  
  //pipe_fd_child2parent[i][0] will hold the file descriptor for reading from the pipe (from child to parent) 
  //pipe_fd_child2parent[i][1] will hold the file descriptor for writing to the pipe (from parent to child).
  for (int i = 0; i < number_of_processes; i++)
    {
        if (pipe(pipe_fd_child2parent[i]) == -1) {           
            perror("pipe");
            exit(1);
        }
    }

  //This block of code is responsible for creating child processes using the fork() system call
  //and setting up communication between each child process and the parent process using pipes.
  for (int i = 0; i < number_of_processes; i++)
  {
        pid_t p_id = fork();
        //fork(): The fork() system call is used to create a new process. After a successful fork(), 
        //two identical processes are created: the parent process and the child process. 
        //In the child process, fork() returns 0, while in the parent process, it returns the process ID (p_id) of the child.
        if(p_id<0){perror("fork"); return 1;}
        
        else if (p_id == 0) //current process is the child process. 
        {
            int val;  
            close(pipe_fd_child2parent[i][0]); //The child won't be reading from this pipe.
            close(pipe_fd_parent2child[i][1]); //The child won't be writing to this pipe.
            while(1)                              
            {
                if(read(pipe_fd_parent2child[i][0], &val, sizeof(int))==-1) //read integer from pipe
                    printf("Error reading from parent\n");
                else
                    printf("[Child %d] [%d] Child received %d!\n", i, getpid(), val);
                val--;
                sleep(10);
                if(write(pipe_fd_child2parent[i][1], &val, sizeof(int))==-1) //write incremented integer to pipe
                    printf("Error sending to parent from child %d\n",i);
                else
                    printf("[Child %d] [%d] Child Finished hard work, writing back %d\n", i, getpid(), val);
            }
        }
        else 
        {
        p_id_array[i] = p_id;
        }
       
            //int close(int fd); 
            //
            //system call
            //closes file descriptor
            //Any further attempts to read from or write to the closed file descriptor will fail.
            //
            //Return Value:
            //
            //On successful completion returns 0.
            //On failure returns -1
            //
            //
            //******************************************************************************
            //
            //ssize_t read(int fd, void *buf, size_t count); 
            //
            //system call
            //is used to read data from an open file descriptor into a buffer
            //
            //Parameters:
            //fd: The file descriptor from which data will be read.
            //buf: A pointer to the buffer where the data will be stored.
            //count: The maximum number of bytes to read.
            //
            //Return Value:
            //On successful completion returns the number of bytes read.
            //If the writing end of a pipe or socket has been closed returns 0.
            //On failure, it returns -1
            //
            //** ssize, size_t:int data types for handling memeory
            //
            //******************************************************************************
            //
            //ssize_t write(int fd, const void *buf, size_t count);
            //
            //system call
            //is used to write data from buffer to open file descriptor
            //Parameters:
            //fd: The file descriptor to which data will be written.
            //buf: A pointer to the buffer containing the data to be written.
            //count: The number of bytes to write from the buffer.
            //
            //Return Value:
            //On successful completion returns the number of bytes written.
            //On failure, it returns -1
            //
            
    }
    int job;                  
    //fd_set read_set;        //a file descriptor set used with select() to monitor file descriptors for reading.    
    int child_iteration = 0;//for selecting next fd in round robin scheduling.  

    for(int i = 0; i<number_of_processes; i++)
    {
      close(pipe_fd_child2parent[i][1]); //write end of the pipe from child to parent is closed
      close(pipe_fd_parent2child[i][0]); //read end of the pipe from parent to child is closed
      //parent process will only be reading from pipe_fd_child2parent and writing to pipe_fd_parent2child, 
      //so it doesn't need these file descriptor ends open.
    }
   
   
  
   
   //In this block of code parent process manages communication with the child processes using the select() function.  
   while (1)
   {
        //This part of the code effectively blocks until activity occurs on one of the file descriptors being monitored.
        // Upon activity, it returns with information about which file descriptors are ready, allowing the program 
        //to take appropriate action.
        struct pollfd fds[number_of_processes +1];
        //pollfd: represents file descriptor
        //n_o_p+1: struct holds all file decriptors needed
                                  
        for (int i = 0; i < number_of_processes; i++)    
        {                                                 
            fds[i].fd=pipe_fd_child2parent[i][0];//assigns the file descriptor for reading from the pipe 
            fds[i].events = POLLIN;//data is to be read
        }
        
        //input from terminal
        fds[number_of_processes].fd = STDIN_FILENO; // stdin file descriptor
        fds[number_of_processes].events = POLLIN; // Set to poll for incoming data
         
        //int poll(struct pollfd fds[], nfds_t nfds, int timeout);
        //
        //system call
        //
        //Parameters:
        //
        //fds[]: An array of struct pollfd structures, each describing a file descriptor to monitor and the events to wait for
        //nfds: The number of elements in the fds array.
        //timeout: The maximum time, in milliseconds, that poll() should wait for an event., -1->infinite timeout
        //
        //Return Value:
        //On success, poll() returns the number of file descriptors with events ready.
        //If no events occur before the timeout (if specified) or if poll() is interrupted by a signal, it returns 0.
        //On error, it returns -1, and errno is set to indicate the error.
        
        
      
        int retval = poll(fds, number_of_processes + 1, -1); // Infinite timeout
        if (retval < 0) {
        perror("poll");
        return 1;
        }
        
        //This part of the code handles the scenario where input is available from the standard input 
        //if (FD_ISSET(0, &read_set))//if input is available
        
        
        if(fds[number_of_processes].revents & POLLIN)
        // checks if there is data available for reading on the standard input file descriptor.
        //fds[number_of_processes].revents: accesses the bitmask of events that occurred on the standard input file descriptor.
        // check if any events occurred on the standard input file descriptor after calling the poll() function
        //POLLIN: data is available for reading on the fd
        
        
        {
            fgets(buffer,BUFFER_SIZE,stdin);
            
            //char *fgets(char *str, int n, FILE *stream);
            //
            //function
            // 
            //str: Pointer to an array where the string read is stored
            //n: Maximum number of characters to be copied into str
            //stream: Pointer to a FILE object that identifies the stream from which to read.
            //In this case, stdin is usually passed to read from the standard input.
            //stores read characters to buffer
            
            
            if (strcmp(buffer,"exit\n") == 0) //if input is "exit"
            {
                
                int number = number_of_processes;
                int status;
                for (int i = 0; i < number_of_processes; i++)
                {
                    if(i==number_of_processes-1)
                        printf("Waiting for %d child to exit\n",number);
                    else
                        printf("Waiting for %d children to exit\n",number);
                    kill(p_id_array[i],SIGTERM);
                    //
                    //kill(pid, sig) system call
                    //sends signal to progress
                    //here: terminate gracefully
                    //
                    --number;
                    //wait(&status)
                    //waits for process to terminate, saves termination status on status pointer
                    //returns terminated process pid or -1 if error
                    if(wait(&status) == -1) 
                    {
                        printf("There are no child processes to wait for\n");
                    }
                }
                printf("All children terminated\n");
                exit(0);
            }
            else if (strcmp(buffer,"help\n") == 0)
            {
                printf("Type a number to send a job to a child\n");
            }
            else  
            {
                char *endptr;
                long num = strtol(buffer,&endptr,10); //convert buffer to long integer,
                //endptr will point to the first character in buffer that could not be converted to a number. 
                //The third argument 10 specifies base 10, indicating that the string represents a decimal number.
                //long int strtol(const char *str, char **endptr, int base);

                if (endptr == buffer || *endptr != '\n' || errno == ERANGE) 
                //no valid int found||string is not fully read||overflow
                {
                    printf("Type a number to send a job to a child\n");
                }
                else
                {
                    job = atoi(buffer);
                    if(round_robin_scheduling)
                    {
                        if(child_iteration == number_of_processes)
                        {
                            child_iteration = 0;//reset cycle
                        }        
                        printf("[Parent] Assigned job %d to child %d\n", job, child_iteration);
                        if(write(pipe_fd_parent2child[child_iteration][1], &job, sizeof(int))==-1)
                        //writes the value of the job variable to the write end of the pipe corresponding to the child process
                            printf("Error sending to child %d", child_iteration);
                        child_iteration++;
                    }

                    if(random_scheduling)
                    {
                        srand(time(NULL));//seed for rand()
                        child_iteration = rand()%number_of_processes;      
                        printf("[Parent] Assigned job %d to child %d\n", job, child_iteration);
                        if(write(pipe_fd_parent2child[child_iteration][1], &job, sizeof(int))==-1)
                            printf("Error sending to child %d", child_iteration);    
                    }
                }
            }
        }
        
         
            for (int i = 0; i < number_of_processes; i++) 
            {
                if (fds[i].revents & POLLIN)     //We iterate over the file descriptors of the child channel
                {
                    if(read(pipe_fd_child2parent[i][0], &job, sizeof(int)) == -1)
                        {perror("read"); return 1;}
                    else
                        printf("[Parent] Value received from child %d: %d\n", i, job);
                }  
            }
        
        
    }
 return 0;
}
