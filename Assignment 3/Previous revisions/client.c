#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <fstream>


#define MAXLINE 4096 /*max text line length*/
//#define SERV_PORT 10010 /*port*/

//This method to return the current date and time.
char *getPDate(char *input){
  FILE *pp;
  char *line;

  pp = popen(input, "r");
  if (pp != NULL) {
      char buf[1000];
      line = fgets(buf, sizeof buf, pp);
    pclose(pp);
  }
  
  return line;
}


void sleep3(){

  printf("%s", "Going to sleep for 3 seconds.\n");
  int count = 1;
  while(count <= 3){
    sleep(1);
    printf("Please wait...\n");
    count++;
  }
 	std::ofstream outFile;
  outFile.open("a3p1Client.log", std::ios_base::app);
  outFile << "------------------------------------------------------" << std::endl;
  outFile << "********** Begin Client Log" << " **********\n" << std::endl;
  
    char *nowDate = getPDate("date");
    
        printf("%s", "Current date and time is: ");
        printf("%s", nowDate);
        
        char tempid[28];
        for(int ii = 0; ii < 28; ii++){
          tempid[ii] = nowDate[ii];
        }
   outFile << "Current date and time is: ";
        
        for(int ii = 0; ii < 28; ii++){
          outFile << tempid[ii];
        }
        
   outFile << std::endl;
     
        char *curpid = getPDate("echo $$");
        printf("%s", "Process ID is: ");
        printf("%s", curpid);
        
  outFile << "Process ID is " << curpid << std::endl;
  
  outFile.close();
  
  printf("%s", "Ready to take command:\n");
}

int
main(int argc, char **argv) 
{
 int sockfd;
 struct sockaddr_in servaddr;
 char sendline[MAXLINE], recvline[MAXLINE];

 // alarm(300);  // to terminate after 300 seconds
	
 //basic check of the arguments
 //additional checks can be inserted
 if (argc !=3) {
  perror("Usage: TCPClient <Server IP> <Server Port>"); 
  exit(1);
 }
	
 //Create a socket for the client
 //If sockfd<0 there was an error in the creation of the socket
 if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
  perror("Problem in creating the socket");
  exit(2);
 }
	
 //Creation of the socket
 memset(&servaddr, 0, sizeof(servaddr));
 servaddr.sin_family = AF_INET;
 servaddr.sin_addr.s_addr = inet_addr(argv[1]);
 servaddr.sin_port = htons((int) strtol(argv[2], (char **)NULL, 10)); 
 //servaddr.sin_port =  htons(SERV_PORT); //convert to big-endian order
	
 //Connection of the client to the socket 
 if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) {
  perror("Problem in connecting to the server");
  exit(3);
 }
     char *nowDate = "";
     printf("%s", nowDate);
     
     	std::ofstream outFile;
     outFile.open("a3p1Client.log", std::ios_base::app);
  outFile << "------------------------------------------------------" << std::endl;
  outFile << "********** Begin Client Log" << " **********\n" << std::endl;
  
     nowDate = getPDate("date");
    
        printf("%s", "Current date and time is: ");
        printf("%s", nowDate);
        
        char tempid[28];
        for(int ii = 0; ii < 28; ii++){
          tempid[ii] = nowDate[ii];
        }
   outFile << "Current date and time is: ";
        
        for(int ii = 0; ii < 28; ii++){
          outFile << tempid[ii];
        }
        
   outFile << std::endl;
     
        char *curpid = getPDate("echo $$");
        printf("%s", "Process ID is: ");
        printf("%s", curpid);
        
  outFile << "Process ID is " << curpid << std::endl;
  printf("%s", "Ready to take command:\n");
  
  outFile.close();

 while (fgets(sendline, MAXLINE, stdin) != NULL) {

  send(sockfd, sendline, strlen(sendline), 0);
		
  if (recv(sockfd, recvline, MAXLINE,0) == 0){
   //error: server terminated prematurely
   perror("The server terminated prematurely"); 
   exit(4);
  }else{
   	std::ofstream outFile;
  outFile.open("a3p1Client.log", std::ios_base::app);
 
  printf("%s", "Result received from the server:\n");
  fputs(recvline, stdout);
  printf("%s", "\n");
  
  outFile << "Command sent was: " << sendline << std::endl;
  
  outFile << "Result received from the server: " << std::endl;
  outFile << recvline << std::endl;
  outFile << "********** End Client Log  " << " **********\n";
  outFile << "------------------------------------------------------\n" << std::endl;
   
  outFile.close();
        
      sleep3();
      
  }
 }

 exit(0);
}
