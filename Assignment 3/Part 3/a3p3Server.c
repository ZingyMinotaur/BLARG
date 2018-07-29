#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <cstdlib>
#include <unistd.h> 
#include <algorithm>

pthread_mutex_t lock;

/*
    CONCURRENT SERVER: THREAD EXAMPLE
    Must be linked with the "pthread" library also, e.g.:
    g++ -o server server.c -lnsl -lpthread 

    This program creates a connection socket, binds a name to it, then
    listens for connections to the sockect.  When a connection is made,
    it accepts messages from the socket until eof, and then waits for
    another connection...

    This is an example of a CONCURRENT server -- by creating threads several
    clients can be served at the same time...

    This program has to be killed to terminate, or alternately it will abort in
    120 seconds on an alarm...
*/

//This method to run command and save to a file to be read later. The return is not used.
int runcmd(char *input){
	std::ofstream outFile;
 	outFile.open("temp.txt", std::ios_base::app);
 	FILE *pp;
 	char *line;
 	int length = 0;
  //Remove the newline character from user input (from client).
  *std::remove(input, input+strlen(input), '\n') = '\0';
  //If there is an error, send the error message to errtemp.txt
  //Otherwise, proceed as normal and output result of temp.txt
  char *temp2 = " 2> errtemp.txt";
  char *temp = (char *) malloc(1 + strlen(input)+ strlen(temp2) );
  strcpy(temp, input);
  strcat(temp, temp2);

 	pp = popen(temp, "r");
 	if (pp != NULL) {
    while (1) {
      char buf[1000];
      line = fgets(buf, sizeof buf, pp);
      if (line == NULL) break;
      else {
      outFile << line;
      length = length + strlen(line);
      }
    }
    pclose(pp);
  }
  delete[] line;
  outFile.close();
  
  return length;
}

//This method to return the current date or the PID
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

//This method to append to server log file.
void appServer(char *input){
	//Open the server log file.
	std::ofstream outFile;
 	outFile.open("a3p1Server.log", std::ios_base::app);
 
 	//Print the passed value into server log file
 	outFile << input;
 	
 	//Close server log file.
 	outFile.close();
}

struct serverParm {
    int connectionDesc;
};

void *serverThread(void *parmPtr) {

#define PARMPTR ((struct serverParm *) parmPtr)
    pthread_mutex_lock(&lock);
    
    int recievedMsgLen;
    char messageBuf[1025];
    int i = 0;

    /* Server thread code to deal with message processing */
    printf("DEBUG: connection made, connectionDesc=%d\n",
            PARMPTR->connectionDesc);
    if (PARMPTR->connectionDesc < 0) {
        printf("Accept failed\n");
        return(0);    /* Exit thread */
    }
    
    
    /* Receive messages from sender... */
    while ((recievedMsgLen=
            read(PARMPTR->connectionDesc,messageBuf,sizeof(messageBuf)-1)) > 0) 
    {
        recievedMsgLen[messageBuf] = '\0';
        
        //Open the server log file
        std::ofstream outFile;
 	    outFile.open("a3p1Server.log", std::ios_base::app);
        
 	    //Get the current date and print to console.
        char *nowDate = getPDate("date");
        printf("%s", "Current date and time is: ");
        printf("%s", nowDate);
        
        char tempid[28];
        for(int ii = 0; ii < 28; ii++){
          tempid[ii] = nowDate[ii];
        }
        int clientNum = PARMPTR->connectionDesc - 3;   
        //Print the current date and client number to server log file.
        outFile << "------------------------------------------------------" << std::endl;
        outFile << "********** Client number is " << clientNum << " **********\n" << std::endl;
        outFile << "Current date and time is: ";
        
        for(int ii = 0; ii < 28; ii++){
          outFile << tempid[ii];
        }
        outFile << std::endl;
     
        //Get the current PID and print to console.
        char *curpid = getPDate("echo $$");
        printf("%s", "Process ID is: ");
        printf("%s", curpid);
        
        //Now print the PID to the server log file.
        outFile << "Process ID is " << curpid << std::endl;
        
        //Print to console the client command.
        printf("Client %d command is: %s\n",clientNum,messageBuf);
        
        //Print to server log file the client command and end the log.
        outFile << "Client command is: " << messageBuf << std::endl;
        
        outFile << "**********  END OF LOG **********\n";
        outFile << "------------------------------------------------------\n" << std::endl;
        
        int length = runcmd(messageBuf);
        char temp[4096];
        
        //Close the server log file.
        outFile.close();
        
        //First check to see if command was successful 
        //If the errtemp file is empty, proceed to extract the output from temp file
        //Else, if the errtemp file has something, get contents of errtemp (command failed)
              std::ifstream errFile("errtemp.txt", std::ios::in);
              if ( errFile.peek() == std::ifstream::traits_type::eof() ){
               //Now get data from the temp file created from the runcmd method
              //This contains the result of the client command, which is limited to 1025 characters
              //Make sure to close the errtemp file that we used to check if there was a failed command.
              errFile.close();
              std::ifstream tempFile ("temp.txt", std::ios::in);
        
              if (tempFile.is_open())
              {
              	while (!tempFile.eof())
              	{
        	      	tempFile.get(temp[i]);
        		      i++;
        	      }  
              }
              tempFile.close();
            }
            else{
              //Now get data from the temp file created from the runcmd method
              //This contains the result of the client command,
              if (errFile.is_open())
              {
              	while (!errFile.eof())
              	{
        	      	errFile.get(temp[i]);
        		      i++;
        	      }  
              }
              errFile.close();
              
           }//Closing bracket for else statement to get the contents of temp file (valid command)
        // }//Closing bracket for if statement to check if the errtemp file exists (should always exist)

        //Send the result of the client command to the client.
        if (write(PARMPTR->connectionDesc, temp , 4096) < 0) {
               perror("Server: write error");
               return(0);
        }
              
        //Now delete the temp file and clear the temp array.
        length = 0;
        i = 0;
        memset(temp, 0, sizeof(temp));
        remove("temp.txt");            
    }

  
    close(PARMPTR->connectionDesc);  /* Avoid descriptor leaks */
    free(PARMPTR);                   /* And memory leaks */
    pthread_mutex_unlock(&lock);
    return(0);                       /* Exit thread */
}

//Get the user input from console. if the second argument is a valid port, continue. 
main(int argc, char **argv) {
    int listenDesc;
    struct sockaddr_in myAddr;
    struct serverParm *parmPtr;
    int connectionDesc;
    pthread_t threadID;

    /* For testing purposes, make sure process will terminate eventually */
    alarm(120);  /* Terminate in 120 seconds */

    /* Create socket from which to read */
    if ((listenDesc = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("open error on socket");
        exit(1);
    }

    /* Create "name" of socket */
    myAddr.sin_family = AF_INET;
    //Set the network address as any (if local, 127.0.0.1).
    myAddr.sin_addr.s_addr = INADDR_ANY;
    //Set the port to the user argument (second argument).
    myAddr.sin_port = htons((int) strtol(argv[1], (char **)NULL, 10)); 
        
    if (bind(listenDesc, (struct sockaddr *) &myAddr, sizeof(myAddr)) < 0) {
        perror("bind error");
        exit(1);
    }

    /* Start accepting connections.... */
    /* Up to 5 requests for connections can be queued... */
    listen(listenDesc,5);

    while (1) /* Do forever */ {
        /* Wait for a client connection */
        connectionDesc = accept(listenDesc, NULL, NULL);

        /* Create a thread to actually handle this client */
        parmPtr = (struct serverParm *)malloc(sizeof(struct serverParm));
        parmPtr->connectionDesc = connectionDesc;
        if (pthread_create(&threadID, NULL, serverThread, (void *)parmPtr) 
              != 0) {
            perror("Thread create error");
            close(connectionDesc);
            close(listenDesc);
            exit(1);
        }

        printf("Parent ready for another connection\n");
    }
    pthread_mutex_destroy(&lock);

}
