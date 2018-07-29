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

/*
    CONCURRENT SERVER: THREAD EXAMPLE
    Must be linked with the "pthread" library also, e.g.:
       cc -o example example.c -lnsl -lsocket -lpthread 

    This program creates a connection socket, binds a name to it, then
    listens for connections to the sockect.  When a connection is made,
    it accepts messages from the socket until eof, and then waits for
    another connection...

    This is an example of a CONCURRENT server -- by creating threads several
    clients can be served at the same time...

    This program has to be killed to terminate, or alternately it will abort in
    120 seconds on an alarm...
*/

//This method to run command and save to a file. Return the length of the file.
int runcmd(char *input){
	std::ofstream outFile;
 	outFile.open("temp.txt", std::ios_base::app);
  FILE *pp;
  char *line;
  int length = 0;
  
//  char temp[] = "ls -l | grep \"cli\"";

  pp = popen(input, "r");
  if (pp != NULL) {
    while (1) {
      char buf[1000];
      line = fgets(buf, sizeof buf, pp);
      if (line == NULL) break;
      else {
   //   printf("%s", line); /* line includes '\n' */
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

//This method to return the current date and time..
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
	std::ofstream outFile;
 	outFile.open("a3p1Server.log", std::ios_base::app);
 
  outFile << input;
  outFile.close();
}/*
//This method to append to server log file.
void appServer(char input[5]){
	std::ofstream outFile;
 	outFile.open("a3p1Server.log", std::ios_base::app);
 
  outFile << input;
  outFile.close();
}
*/
struct serverParm {
           int connectionDesc;
       };

void *serverThread(void *parmPtr) {

#define PARMPTR ((struct serverParm *) parmPtr)
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
        
        //Get the current PID and also get the current date (to output as heading)
        	std::ofstream outFile;
 	        outFile.open("a3p1Server.log", std::ios_base::app);
        
        char *nowDate = getPDate("date");
        printf("%s", "Current date and time is: ");
        printf("%s", nowDate);
        
        char tempid[28];
        for(int ii = 0; ii < 28; ii++){
          tempid[ii] = nowDate[ii];
        }
        
        outFile << "------------------------------------------------------" << std::endl;
        outFile << "********** Client number is " << PARMPTR->connectionDesc << " **********\n" << std::endl;
        outFile << "Current date and time is: ";
        
        for(int ii = 0; ii < 28; ii++){
          outFile << tempid[ii];
        }
        outFile << std::endl;
     
        char *curpid = getPDate("echo $$");
        printf("%s", "Process ID is: ");
        printf("%s", curpid);
        
        outFile << "Process ID is " << curpid << std::endl;
        
        printf("Client command is: %s\n",messageBuf);
        
        outFile << "Client command is: " << messageBuf << std::endl;
        
        outFile << "**********  END OF LOG **********\n";
        outFile << "------------------------------------------------------\n" << std::endl;
        
        int length = runcmd(messageBuf);
        char temp[4096];
        
        outFile.close();
        
        //Now get data from the temp file created from previous command.
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
                 
                 //Now append to the server log file.
                 
    }

  
    close(PARMPTR->connectionDesc);  /* Avoid descriptor leaks */
    free(PARMPTR);                   /* And memory leaks */
    return(0);                       /* Exit thread */
}

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
    myAddr.sin_addr.s_addr = INADDR_ANY;
    //myAddr.sin_port = htons(PORTNUMBER);
//    myAddr.sin_addr.s_addr = inet_addr(argv[1]);
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

}
