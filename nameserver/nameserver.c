/**
 * Author: James Spiares
 * CSCE 3530 Project 2 Solution
 * Implementation for nameserver process
 **/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>
#include <errno.h>
#include "network.h"

int i;

struct Service service, request;

int namesockfd, newnamesockfd, newsockfd;
int setAvailable(struct in_addr);
int setUnavailable(struct in_addr);
uint32_t getAvailableServerAddress();

void catcher(int);
void namecatcher(int);
void newnamecatcher(int);

int main(int argc, char **argv)
{	
	if (argc < 2)	//Incorrect usage
	{
		printf("Usage: \"./name address1 address2 address3 ...\"\n");
		printf("The first %d IP addresses from this list will be stored.\n\n", LISTSIZE);
		exit(1);
	}
	
	static struct sigaction act;	//Set up SIGINT signal handler to stop children processes
	act.sa_handler = catcher;
	sigfillset(&(act.sa_mask));
	sigaction(SIGPIPE, &act, NULL);
	
	service.type = 1;	//Fill service response
	service.flag = 0;
	service.numAddresses = (LISTSIZE < argc - 1 ? LISTSIZE : argc - 1);
	struct in_addr addr;
	for (i = 0; i < (LISTSIZE < argc - 1 ? LISTSIZE : argc - 1); i++)	//Copy addresses from command line arguments
	{
		inet_aton(argv[i + 1], &addr);
		service.serviceAddress[i] = addr.s_addr;
	}
	
	int sockfd;
	struct sockaddr_in server = {AF_INET, htons(DEFAULT_NAME_PORT), htonl(INADDR_ANY)};	//Port 12345
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)	//Create socket
	{
		printf("Failed to create socket.\n\n");
		exit(2);
	}
	if (bind(sockfd, (struct sockaddr *) &server, sizeof(struct sockaddr)) == -1)	//Bind socket
	{
		printf("Failed to connect to socket.\n\n");
		exit(3);
	}
	if (listen(sockfd, 5) == -1)	//Listen to socket
	{
		printf("Failed to listen to socket.\n\n");
		exit(4);
	}
	
	printf("Awaiting connections...\t[Press [^C] to exit.\n");
	while (1)	//Accepting loop
	{
		if ((newsockfd = accept(sockfd, NULL, NULL)) == -1)	//Error
		{
			printf("Failed to accept incoming connection.\n");
			continue;
		}
		if (recv(newsockfd, &request, sizeof(struct Service), 0) > 0)	//Main receiving request loop
		{
			if (request.type == 0)	//Handle request for service address
			{
				printf("Gave service addresses to client.\n");
				send(newsockfd, &service, sizeof(struct Service), 0);	//Send response
			}
		}
		close(newsockfd);
	}
}

void catcher(int signal)
{
	close(newsockfd);
}