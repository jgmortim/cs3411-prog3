/* ----------------------------------------------------------- */
/* NAME : John Mortimroe                     User ID: jgmortim */
/* DUE DATE : 03/31/2019                                       */
/* PROGRAM ASSIGNMENT 3                                        */
/* FILE NAME : reserver.c                                      */
/* PROGRAM PURPOSE :                                           */
/*    The server to receive commands from the client(s).       */
/* ----------------------------------------------------------- */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h> 


int setupServer();
void printPort(int port);
void buildArg();
int runCommand(int fd);
void cleanup();
void errorPrint(char* error);

char **arg;
char *recvBuff;
int argCount=2, maxArgLen=100;
int connfd=0, listenfd=0;

/* ----------------------------------------------------------- */
/* FUNCTION : main                                             */
/*    The main method for the program.                         */
/* PARAMETER USAGE :                                           */
/*    N/A                                                      */
/* FUNCTIONS CALLED :                                          */
/*    accept(), atoi(), buildArg(), cleanup(), malloc(),       */
/*    sizeof(), setupServer(), realloc(), recv(), runCommand() */
/* ----------------------------------------------------------- */
int main() {
	int length, i=0, recvLen=0;
	struct sockaddr_in clnt_addr; 
	/* Set op the server. */
	if ((i=setupServer())!=0) return i;
	length = sizeof(clnt_addr);

	while(1) {
		connfd = accept(listenfd, (struct sockaddr*)&clnt_addr, (socklen_t*) &length); 
		/* recieve the length of the command to run. */
		recvBuff = malloc(sizeof(char)*5);
		for (i=0; i<4; i++) {						
			recv(connfd, &recvBuff[i], 1, 0);
		}
		recvBuff[i]='\0'; 	
		recvLen = atoi(recvBuff);
		/* Allocate space for the command. */
		recvBuff = realloc(recvBuff, sizeof(char)*(recvLen+10));
		/* Receive the command. */
		for (i=0; i<recvLen; i++) {						
			recv(connfd, &recvBuff[i], 1, 0);
		}
		recvBuff[i]='\0'; 
		/* Build the argument array and run the command. */
		buildArg();
		if ((i=runCommand(connfd)) != 0) return i;
		cleanup();
     }
}

/* ----------------------------------------------------------- */
/* FUNCTION : setupServer                                      */
/*    Does all of the set up for the server.                   */
/* PARAMETER USAGE :                                           */
/*    N/A                                                      */
/* FUNCTIONS CALLED :                                          */
/*    bind(), bzero(), getsocketname(), htonl(), htons(),      */
/*    listen(), ntohs(), printPort(), sizeof(), socket()       */
/* ----------------------------------------------------------- */
int setupServer() {
	int length;
	struct sockaddr_in serv_addr; 
	/* Create the socket. */
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		errorPrint("Error: Could not create socket\n");
		return 4;
	} 
	bzero((char *) &serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(0); /* Get a random port. */
	/* Attempt to bind. */
	if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1){
		errorPrint("Error: Bind() failed\n");
		return 5;
	} 
	length = sizeof(serv_addr);
	getsockname(listenfd, (struct sockaddr *)&serv_addr, (socklen_t*) &length);
	printPort(ntohs(serv_addr.sin_port));
	/* Listen for connections. */
	if (listen(listenfd, 10) == -1) {
		errorPrint("Error: listen() failed\n");
		return 6;
	}
	return 0;
}

/* ----------------------------------------------------------- */
/* FUNCTION : printPort                                        */
/*    Prints the port number.                                  */
/* PARAMETER USAGE :                                           */
/*    port {int} - the port number.                            */
/* FUNCTIONS CALLED :                                          */
/*    sprintf(), strlen(), write()                             */
/* ----------------------------------------------------------- */
void printPort(int port) {
	char out[100];
	sprintf(out, "reserver:: listening on port number %d\n", port);
	write(1, out, strlen(out));
}

/* ----------------------------------------------------------- */
/* FUNCTION : runCommand                                       */
/*    forks a new process to run the command and redirects the */
/*    output to client. Pluss error checking.                  */
/* PARAMETER USAGE :                                           */
/*    fd {int} - file descriptor for the client.               */
/* FUNCTIONS CALLED :                                          */
/*    dup2(), errorPrint(), execvp(), fork()                   */
/* ----------------------------------------------------------- */
int runCommand(int fd) {
	int status;
	int pid=fork();
	if (pid == -1) {
		errorPrint("Error: failed to create child process.\n");
		return 1;
	} else if (pid == 0) {
		if (dup2(fd, 1) == -1) { /* Redirect stdout. */
			errorPrint("Error: output redirection failed.\n");
			return 2;
		}
		if (dup2(fd, 2) == -1) { /* Redirect stderr. */
			errorPrint("Error: output redirection failed.\n");
			return 2;
		}
		if (execvp(arg[0], arg) == -1) { /* Run the command. */
			errorPrint("Server Error: command execution failed.\n");
			return 3;
		}
	}
	wait(&status);
	return 0;
}

/* ----------------------------------------------------------- */
/* FUNCTION : buildArg                                         */
/*    Creates the arg array for execvp.                        */
/* PARAMETER USAGE :                                           */
/*    N/A                                                      */
/* FUNCTIONS CALLED :                                          */
/*    malloc(), sizeof(), sprintf(), strtok()                  */
/* ----------------------------------------------------------- */
void buildArg() {
	int i, j;
	char *delim;
	arg=malloc(sizeof(char*)*argCount); /* Default 2 argument. */
	for(i=0; i<argCount; i++) {
		arg[i]=malloc(sizeof(char)*maxArgLen); /* Max length is 100 chars. */
	}
	delim = strtok(recvBuff, ",");
	i=0;
	while (delim != NULL) {
		sprintf(arg[i++], "%s", delim);
		delim = strtok(NULL, ",");
		/* Allocate space for more arguments. */
		if (i==argCount) {
			argCount += 10;
			arg = realloc(arg, sizeof(char*)*argCount);
			for(j=i; j<argCount; j++) {
				arg[j]=malloc(sizeof(char)*maxArgLen);
			}
		}
	}
	free(arg[i]);
	arg[i]=NULL;
}

/* ----------------------------------------------------------- */
/* FUNCTION : cleanup                                          */
/*    Frees up any allocated memory and closes sockets.        */
/* PARAMETER USAGE :                                           */
/*    N/A                                                      */
/* FUNCTIONS CALLED :                                          */
/*    close(), free()                                          */
/* ----------------------------------------------------------- */
void cleanup() {
	int i;		
	for(i=0; i<argCount; i++) {
		if (arg[i]!=NULL) free(arg[i]);
	}
	if (arg!=NULL) free(arg);
	if (recvBuff!=0) free(recvBuff);
	close(connfd);
}

/* ----------------------------------------------------------- */
/* FUNCTION : errorPrint                                       */
/*    prints error message and cleanup so program can exit.    */
/* PARAMETER USAGE :                                           */
/*    error {char*} - the error message to print.              */
/* FUNCTIONS CALLED :                                          */
/*    cleanup(), strlen(), write()                             */
/* ----------------------------------------------------------- */
void errorPrint(char* error) {
	write(1, error, strlen(error));
	cleanup();
}
