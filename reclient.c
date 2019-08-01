/* ----------------------------------------------------------- */
/* NAME : John Mortimroe                     User ID: jgmortim */
/* DUE DATE : 03/31/2019                                       */
/* PROGRAM ASSIGNMENT 3                                        */
/* FILE NAME : reclient.c                                      */
/* PROGRAM PURPOSE :                                           */
/*    The client to send commands to the server.               */
/* ----------------------------------------------------------- */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 

int setupClient(char* host, int port);
void sendToServer();
void recvFromServer();
void cleanup();
void errorPrint(char* error);

char *command, *packet;
int sockfd=0;

/* ----------------------------------------------------------- */
/* FUNCTION : main                                             */
/*    The main method for the program.                         */
/* PARAMETER USAGE :                                           */
/*    argc {int} - number of argument.                         */
/*    argv {char**} - list of arguments.                       */
/* FUNCTIONS CALLED :                                          */
/*    atoi(), cleanup(), errorPrint(), malloc(),               */
/*    recvFromServer(), sendToServer(), setupClient(),         */
/*    strcat(), strcpy(), strlen()                             */
/* ----------------------------------------------------------- */
int main(int argc, char *argv[]) {
	int i, port, comLen=0;
	char *host;
	if(argc < 4) {
		errorPrint("Usage:\n reclient <address> <port> <command> [args]...\n");
		return 1;
	}
	/* Get the host and port. */
	host = argv[1];
	port = atoi(argv[2]);
	/* Find total length of all additional args. */
	for (i=3; i<argc; i++) {
		comLen += strlen(argv[i]);
	}
	comLen += i*2; 
	/* Allocate enough space for a single string to hold all commands and args. */
	command = malloc(sizeof(char)*(comLen));
	strcpy(command, argv[3]);
	for (i=4; i<argc; i++) {
		strcat(command, ",");
		strcat(command, argv[i]);
	}

	setupClient(host, port);
	sendToServer();
	cleanup();
	recvFromServer();
	return 0;
}

/* ----------------------------------------------------------- */
/* FUNCTION : setupClient                                      */
/*    Sets up the client to communicate with the server.       */
/* PARAMETER USAGE :                                           */
/*    host {char*} - the host to connect to.                   */
/*    port {int} - the port to connect to.                     */
/* FUNCTIONS CALLED :                                          */
/*    malloc(), send(), sizeof(), sprintf(), strlen()          */
/* ----------------------------------------------------------- */
int setupClient(char* host, int port) {
	struct sockaddr_in serv_addr; 
	struct hostent *h;
	/* Create the socket. */
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		errorPrint("Client Error: Could not create socket\n");
		return 2;
	} 
	
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	h = gethostbyname(host);
	bcopy((char *)h->h_addr, (char *)&serv_addr.sin_addr, h->h_length);
	serv_addr.sin_port = htons(port);

	/* Connect to the server. */
	if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		errorPrint("Client Error: Connect Failed\n");
		return 3;
	} 
	return 0;
}

/* ----------------------------------------------------------- */
/* FUNCTION : sendToServer                                     */
/*    Send commands to the server.                             */
/* PARAMETER USAGE :                                           */
/*    N/A                                                      */
/* FUNCTIONS CALLED :                                          */
/*    malloc(), send(), sizeof(), sprintf(), strlen()          */
/* ----------------------------------------------------------- */
void sendToServer() {
	int comLen = strlen(command);
	packet = malloc(sizeof(char)*(comLen+5));
	sprintf(packet, "%04d%s", comLen, command);
	send(sockfd, packet, strlen(packet), 0);
}

/* ----------------------------------------------------------- */
/* FUNCTION : recvFromServer                                   */
/*    Receives content from the server.                        */
/* PARAMETER USAGE :                                           */
/*    N/A                                                      */
/* FUNCTIONS CALLED :                                          */
/*    sizeof(), read(), write()                                */
/* ----------------------------------------------------------- */
void recvFromServer() {
	char recvBuff[1];
	while ( read(sockfd, recvBuff, sizeof(char)) > 0) {
		write(1, recvBuff, sizeof(char));
	}
}

/* ----------------------------------------------------------- */
/* FUNCTION : cleanup                                          */
/*    Frees up any allocated memory.                           */
/* PARAMETER USAGE :                                           */
/*    N/A                                                      */
/* FUNCTIONS CALLED :                                          */
/*    free()                                                   */
/* ----------------------------------------------------------- */
void cleanup() {
	if (packet!=NULL) free(packet);
	if (command!=NULL) free(command);
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
