/**
 * Author: James Spiares
 * CSCE 3530 Project 1 Solution
 * Implementation for server process
 **/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "network.h"

void catcher(int);
int sockfd, newsockfd;

void produceResponse(struct Message *, struct Message *);

/**
 * Runs a math library service server with TCP or UDP at the given port.
 * Default port: 12345
 * Command line usage: "./server < TCP | UDP > [ PORT ]" 
 * Use [^C] to stop a running server process
**/
int main(int argc, char **argv)
{
	if (argc < 2 || argc > 3)	//Incorrect usage
	{
		printf("Usage: \"./server < TCP | UDP > [ PORT ]\"\n\n");
		exit(1);
	}
	
	static struct sigaction act;	//Set up signal handler
	act.sa_handler = catcher;
	sigfillset(&(act.sa_mask));
	sigaction(SIGPIPE, &act, NULL);
	
	if (strcmp(argv[1], "TCP") == 0)	//Run in TCP mode
	{
		struct sockaddr_in server = {AF_INET, argc > 2 ? htons(atoi(argv[2])) : htons(DEFAULT_SERVICE_PORT), htonl(INADDR_ANY)};
		
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
			if (fork() == 0)	//Spawn a child thread to handle new connection
			{
				printf("New connection accepted. Handled by process %d.\n", getpid());	
				struct Message *s = (struct Message *) malloc(sizeof(struct Message));	//Message to send
				struct Message *r =  (struct Message *) malloc(sizeof(struct Message));	//Message to receive
				struct Packet *packet = (struct Packet *) malloc(sizeof(struct Packet));	//Buffer for network transmission
				memset(packet, 0, sizeof(struct Message));	//Set default values
				while (recv(newsockfd, packet, sizeof(struct Packet), 0) > 0)	//Main receiving request loop
				{
					deserialize(packet, r);	//Deserialize request
					printf("%d: received request\n", getpid());
					printBytes(packet);
					printMessage(r);
					if (r->type != 0)	//Not request message; ignore
						continue;
					produceResponse(r, s);
					serialize(s, packet);	//Serialize response
					send(newsockfd, packet, sizeof(struct Packet), 0);	//Send response
				}
				printf("Client terminated connection for process %d.\n", getpid());	//Indicate client has terminated connection
				close(newsockfd);
				exit(0);
			}
			close(newsockfd);	//Parent closes the new network connection immediately
		}
	}
	else if (strcmp(argv[1], "UDP") == 0)	//Run in UDP mode
	{
		int sockfd;
		struct sockaddr_in server = {AF_INET, htons(DEFAULT_SERVICE_PORT), htonl(INADDR_ANY)};
		struct sockaddr_in client;
		int client_len = sizeof(struct sockaddr_in);
		
		if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)	//Create socket
		{
			printf("Failed to create socket.\n\n");
			exit(2);
		}
		if (bind(sockfd, (struct sockaddr *) &server, sizeof(struct sockaddr)) == -1)	//Bind socket
		{
			printf("Failed to connect to socket.\n\n");
			exit(3);
		}
		
		printf("Awaiting requests...\n");
		struct Message *s = (struct Message *) malloc(sizeof(struct Message));
		struct Message *r = (struct Message *) malloc(sizeof(struct Message));
		struct Packet *packet = (struct Packet *) malloc(sizeof(struct Packet));
		while (1)	//Main listening loop
		{
			if (recvfrom(sockfd, packet, sizeof(struct Packet), 0, (struct sockaddr *) &client, &client_len) == -1)	//Receive request
			{
				printf("Error receiving data.\n");
				continue;
			}
			else
			{
				deserialize(packet, r);	//Deserialize request
				printf("Received request from %s\n", inet_ntoa(client.sin_addr));
				printBytes(packet);
				printMessage(r);
				if (r->type != 0)	//Not request message; ignore
					continue;
				produceResponse(r, s);
				serialize(s, packet);	//Serialize response
				if (sendto(sockfd, packet, sizeof(struct Packet), 0, (struct sockaddr *) &client, client_len) == -1)	//Send response
				{
					printf("Error sending data.\n");
					continue;
				}
			}
		}
	}
	else	//Incorrect usage
	{
		printf("Usage: \"server < TCP | UDP > [ PORT ]\"\n\n");
		exit(1);
	}
}

/**
 * Performs the calculation specified in req and fills res accordingly.
 * Input: req - the request message
 *        res - the response message
 **/
 void produceResponse(struct Message *req, struct Message *res)
 {
	memcpy(res, req, sizeof(struct Message));	//Duplicate request for response
	res->type = 1;	//Set as response
	switch(req->opcode)	//Perform the mathematical operation and fill response
	{
		case 1:
			res->result = req->operand1 + req->operand2;
			res->flag = 1;
			break;
		case 2:
			if (req->operand1 < req->operand2)	//Set underflow error
			{
				res->result = 1;
				res->flag = 0;
			}
			else
			{
				res->result = req->operand1 - req->operand2;
				res->flag = 1;
			}
			break;
		case 3:
			res->result = req->operand1 * req->operand2;
			res->flag = 1;
			break;
		case 4:
			if (req->operand2 == 0)	//Set division by zero error
			{
				res->result = 2;
				res->flag = 2;
			}
			else
			{
				res->result = req->operand1 / req->operand2;
				res->flag = 1;
			}
			break;
		default:	//Set unknown operator error
			res->result = 1;
	}
 }

/**
 * Handles SIGPIPE signals, which indicate that a client has closed a connection
 **/
void catcher(int signal)
{
	close(newsockfd);	//Close the connection
	exit(0);
}
