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

//This method to let the client sleep for 3 seconds, then repeats loop.
void sleep3(){
   //Let user know the client will go to sleep. Then loop the sleep command 3 times printing out "please wait" every time.
  printf("%s", "Going to sleep for 3 seconds.\n");
  int count = 1;
  while(count <= 3){
    sleep(1);
    printf("Please wait...\n");
    count++;
  }
}

//This method to print to client log.
void loopcmd(){

  //Open the client log file and print the beginning of the log.
  std::ofstream outFile;
  outFile.open("a3p3Client2.log", std::ios_base::app);
  outFile << "------------------------------------------------------" << std::endl;
  outFile << "********** Begin Client Log" << " **********\n" << std::endl;
  
  //Get the current date and time. and print to console (part of the header for the next loop).
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
 
  //Get the current PID and print to console (part of the header for the next loop).
  char *curpid = getPDate("echo $$");
  printf("%s", "Process ID is: ");
  printf("%s", curpid);
  
  //Print the PID to the client log file.      
  outFile << "Process ID is " << curpid << std::endl;
  
  //Close the client log file.
  outFile.close();
  
  //Let user know client is ready to accept new command (loop repeats).
  printf("%s", "Ready to take command:\n");
}

int main(int argc, char **argv) 
{
 int sockfd;
 struct sockaddr_in servaddr;
 char sendline[MAXLINE], recvline[MAXLINE];

 alarm(300);  // to terminate after 300 seconds
	
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
	
 //Connection of the client to the socket 
 if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) {
  perror("Problem in connecting to the server");
  exit(3);
 }

 //Initialize the nowdate variable with no data.
 char *nowDate = "";
 printf("%s", nowDate);
 
 //Start the output to the log file (As this is the first log, manually coded below). While loop (when user input is not null) will continue future logs).
 std::ofstream outFile;
 outFile.open("a3p3Client2.log", std::ios_base::app);
 outFile << "------------------------------------------------------" << std::endl;
 outFile << "********** Begin Client Log" << " **********\n" << std::endl;

 //Get the current date and save it  
 nowDate = getPDate("date");
    
 //Print out the current date to screen
 printf("%s", "Current date and time is: ");
 printf("%s", nowDate);
        
 //Output the current date to the log file
 char tempid[28];
 for(int ii = 0; ii < 28; ii++){
       tempid[ii] = nowDate[ii];
 }
 outFile << "Current date and time is: ";
        
 for(int ii = 0; ii < 28; ii++){
      outFile << tempid[ii];
 }
        
 outFile << std::endl;
 
 //Get the current PID and print to screen.    
 char *curpid = getPDate("echo $$");
 printf("%s", "Process ID is: ");
 printf("%s", curpid);
        
 //Then print the PID to log file.
 outFile << "Process ID is " << curpid << std::endl;
 printf("%s", "Ready to take command:\n");

 //Close log file.  
 outFile.close();

 int loopcount = 0;
 //Run the command loop 5 times (as each command loop sleeps for 3 seconds) for a total
 //of a minimum of 15 seconds of runtime.
 while (loopcount < 5) {
	//Send the user command to the server.
	//send the command: " date; ls -l junk.txt; touch junk.txt; ls -l junk.txt "
	
	char *cmdList[3] = {"date", "ls -l junk.txt", "rm -f junk.txt"};
	int cmdLength[3] = {4, 14, 14};
	int commandcount = 0;
	
	while(commandcount < 3) {
	send(sockfd, cmdList[commandcount], cmdLength[commandcount], 0);
		
	//If the server terminated prematurely, output message and exit.
 	if (recv(sockfd, recvline, MAXLINE,0) == 0){
   	perror("The server terminated prematurely"); 
   	exit(4);
  	}
	//Else, get the result from the server.
	else{
		//Open the client log file.
   		std::ofstream outFile;
  		outFile.open("a3p3Client2.log", std::ios_base::app);
 		
		//Print the result received from the server to console.
 		printf("%s", "Result received from the server:\n");
 		fputs(recvline, stdout);
  		printf("%s", "\n");
  	
		//Save the command sent to the server and the result received into the client log file.
  		outFile << "Command sent was: " << cmdList[commandcount] << std::endl;
  
  		outFile << "Result received from the server: " << std::endl;
  		outFile << recvline << std::endl;
  		outFile << "********** End Client Log  " << " **********\n";
  		outFile << "------------------------------------------------------\n" << std::endl;
 			
		//Close the client log file  
  		outFile.close();
        
    //Start the loop again.
      	loopcmd();

 		} //Closing bracket for else statement in while loop
 	commandcount = commandcount + 1;
	} //Closing bracket for while loop to run the 3 given commands.
 loopcount = loopcount + 1;
	//Call the sleep3 method. Mainly puts the client to sleep for 3 seconds, then repeats loop.
 sleep3();
 } //Closing bracket for while loop (up to 5 times as described above).

 exit(0);

} //Closing bracket for main class.
