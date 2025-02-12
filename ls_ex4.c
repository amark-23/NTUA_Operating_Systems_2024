/********************************************************************************************************
Communication between two independent processes in two different OS is established through Sockets.
(Socket: endpoint of communication channel) using a server-client protocol. 

Client process connects to a server

--host HOST :Connect to HOST instead of default
--port PORT :Connect to PORT instead of default
--debug     :print data

Terminal commands:

help                  : help message
exit                  : exit program
get                   : get data from server
                        Replies with values from sensor in the form of X YYY ZZZZ WWWWWWWWWW,where
                        X = 0: boot , 1: setup, 2: interval, 3: button, 4: motion
                        YYY = Luminosity
                        ZZZZ = Temperature
                        WWWWWWWWWW = UNIX Timestamp

N name surname reason : sends message (name + reason to go out)
                        response is ACK N name surname reason
********************************************************************************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <time.h>
#include <poll.h>
#define BUF_SIZE 1024

int main(int argc, char **argv)
{
 bool  debug        = false;
 int   PORT         = 20241;
 char* HOST         = "os4.iot.dslab.ds.open-cloud.xyz";
 char  buf[BUF_SIZE];

 /*****************************************************Error Handling*********************************************************/
 if(argc > 6) {printf("Error-Usage : ./ask4 [--host HOST] [--port PORT] [--debug]\n"); exit(1);}
 if(argc > 1) {
    for(int i=1;i<argc;i++) {
        if(!strcmp(argv[i],"--host"))       {HOST = argv[i+1]; i++;}
        else if(!strcmp(argv[i],"--port"))  {PORT = atoi(argv[i+1]); i++;}
        else if(!strcmp(argv[i],"--debug")) {debug = true;}
        else                                {printf("Error-Usage : ./ask4 [--host HOST] [--port PORT] [--debug]\n"); exit(1);}
    }
 }
 /****************************************************************************************************************************/

 /************************************************Socket Initialization*******************************************************/
 int sd = socket(AF_INET, SOCK_STREAM, 0);
 if (sd < 0) {perror("socket"); return -1;}
 /*
 int socket(int domain, int type, int protocol)
 
 creates socket file descriptor
 domain (or Address Family): Specifies the protocol family for the socket(AF_INET, AF_INET6, AF_UNIX,...)
 type:                       Specifies the type of socket(SOCK_STREAM, SOCK_DGRAM,...)
 protocol:                   Specifies the protocol (TCP, UDP, ICMP,...)

 AF_INET     : IPv4
 SOCK_STREAM : TCP(Transmission Control): two way connection based byte stream 
 0           : protocol is chosen by the system  
 */

 struct sockaddr_in addr;      //socket address
 struct hostent *hostp;        //pointer to struct containing host information
 /*
 struct sockaddr_in {
    short            sin_family;   // Address family
    unsigned short   sin_port;     // Port number (in network byte order)
    struct in_addr   sin_addr;     // IPv4 address
    char             sin_zero[8];  // Padding, to make the structure the same size as standard struct sockaddr
 };

 struct in_addr {
    in_addr_t  s_addr;  // 32-bit unsigned integer representing an IPv4 address in network byte order.
 };

 struct hostent {
    char    *h_name;        // Official name of the host
    char    **h_aliases;    // A null-terminated array of alternate names for the host
    int     h_addrtype;     // Address type (e.g., AF_INET for IPv4)
    int     h_length;       // Length of address in bytes
    char    **h_addr_list;  // A null-terminated array of pointers to network addresses
 };
 */

 hostp = gethostbyname(HOST);  //retrieve HOST information
 /*
 gethostbyname(char *name) 
 input           : name of host
 output          : pointer to struct containing host information
 error handling  : if host is not found, returns NULL
 */

 addr.sin_family = AF_INET;    //set address family to IPv4
 addr.sin_port = htons(PORT);  //set port number, htons() converts short integer to network byte order
 
 bcopy(hostp->h_addr_list[0], &addr.sin_addr, hostp->h_length); //set IP address

 /*
 The bcopy() function is used for memory copying.
 This line:
 copies the first address (h_addr_list[0]) from the list of addresses associated with the hostname (retrieved using gethostbyname()) 
 to the sin_addr field of the addr structure.  
 It copies hostp->h_length bytes from the source address (hostp->h_addr_list[0]) to the 
 destination address (&addr.sin_addr).
 
 void bcopy(const void *src, void *dest, size_t n);
 src: A pointer to the source memory location from which data will be copied.
 dest: A pointer to the destination memory location where data will be copied to.
 n: The number of bytes to copy.
*/

 if(connect(sd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {perror("connect"); return -1;}

 /*
 int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
 sockfd: file descriptor of the socket to connect to,obtained from a previous call to socket().
 addr: A pointer to a sockaddr structure that specifies the address of the socket
 addrlen: The size of the address structure pointed to by addr.

 connect(sd, (struct sockaddr *)&addr, sizeof(addr))
 system call, establishes a connection to the specified socket
 */
 
 /****************************************************************************************************************************/

 /********************************************Communication Loop**************************************************************/

 struct pollfd fds[2];
 fds[0].fd = 0;  //stdin
 fds[0].events = POLLIN;
 fds[1].fd = sd; //socket
 fds[1].events = POLLIN;
 
 /*
 struct pollfd {
    int fd;           // File descriptor to monitor
    short events;     // Events to monitor (input, output, errors, etc.)
    short revents;    // Events that occurred (set by poll)
 };
  
  int poll(struct pollfd fds[], nfds_t nfds, int timeout);
  system call, is used to wait for events on multiple file descriptors
  fds[]: An array of struct pollfd structures, each describing a file descriptor to monitor and the events to wait for
  nfds: The number of elements in the fds array.
  timeout: The maximum time, in milliseconds, that poll() should wait for an event., -1->infinite timeout
  Return Value:
    On success, poll() returns the number of file descriptors with events ready.
    If no events occur before the timeout (if specified) or if poll() is interrupted by a signal, it returns 0.
    On error, it returns -1, and errno is set to indicate the error.
    
  POLLIN:data is available  
*/

 while (true) {
     int ret = poll(fds, 2, -1);
     if (ret < 0) {perror("poll"); return 1;}
     
     /************************************************Data from Terminal*************************************************/
     //data available on stdin
     if (fds[0].revents & POLLIN) {
        if(fgets(buf, BUF_SIZE, stdin) == NULL) {perror("fgets"); return -1;}
        
        //exit
        if(!strcmp(buf,"exit\n")) {printf("Exiting...\n"); close(sd); exit(0);}
        
        //help
        if(!strcmp(buf,"help\n")) 
        {printf("Type 'exit' to exit, 'get' to recover server data, 'N name surname reason' to request access to break quarantine.\n");
         continue;
         }
        
        //debug
        if(debug) {
            int i=0;
            while(buf[i] != '\n') ++i;
            buf[i] = '\0';
            printf("[DEBUG] sent '%s'\n",buf);
        }
        
        //write to server
        if(write(sd,&buf,strlen(buf)+1) == -1) {perror("write"); return -1;}
     }
     
     /************************************************Data from Server***************************************************/
     //data available on socket
     else if (fds[1].revents & POLLIN) {
        char output[BUF_SIZE];
        int nbytes = read(sd, &output, BUF_SIZE-1);
          
        if(nbytes == -1) {perror("read"); return -1;}
        output[nbytes-1] = '\0';

        if(debug) printf("[DEBUG] read '%s'\n",output);
        if(!strcmp(output,"try again") || !strcmp(output,"invalid code")){printf("%s\n",output); continue;}

        if(output[1] == ' ') //server sent a reply to a 'get' request
        {   
            //format:X YYY ZZZZ WWWWWWWWWW
            int flag;
            int luminosity;
            int temp;
            int timestamp;
            char* token;
            const char s[2] = " ";
           
            token = strtok(output, s);
            flag = atoi(token);
           
            token = strtok(NULL, s);
            luminosity = atoi(token);
           
            token = strtok(NULL, s);
            temp = atoi(token);
           
            token = strtok(NULL, s);
            timestamp = atoi(token);
            
            char* message;
            double temperature = (double)temp/100;

            time_t senttime = timestamp;
            struct tm *info;
            time( &senttime );
            info = localtime( &senttime );

            switch (flag)
            {
                case 0:
                    message = "boot";
                    break;
                case 1:
                    message = "setup";
                    break;
                case 2:
                    message = "interval";
                    break;
                case 3:
                    message = "button";
                    break;
                case 4:
                    message = "motion";
                    break;
                default:
                    message = "unknown flag";
                    break;
            }
            printf("-----------------------------------\n");
            printf("Latest Event:\n");
            printf("%s (%d)\n",message,flag);
            printf("Temperature : %.2f C\n",temperature);
            printf("Luminosity  : %d\n",luminosity);
            printf("Timestamp   : %s",asctime(info));
            printf("-----------------------------------\n");

      
        }
    
        else if(output[0] == 'A' && output[1] == 'C' && output[2] == 'K') {printf("Response: '%s'\n",output);}
        
        else {printf("Send verification code: '%s'\n",output);}
        
        }

 /****************************************************************************************************************************/
}
return 0;
}
