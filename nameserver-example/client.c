 /**
 * Author: James Spiares
 * CSCE 3530 Project 1 Solution
 * Implementation for client process
 **/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include "network.h"

int sockfd;
int i, numAddresses;
int receivedResponse;
char line[256];
struct sockaddr_in server;
struct timeval timeout;

uint32_t addresses[LISTSIZE];

int promptUser(struct Message *);
void printResult(struct Message *);

int getServiceAddresses();
int establishConnectionTCP(int);
int establishConnectionUDP(int);

/**
 * Runs a math library service client with TCP or UDP at the given address and port.
 * Default address: 126.120.151.97 (cse04.cse.unt.edu)
 * Command line usage: "client < TCP | UDP > [ PORT [ IP_ADDRESS ] ]" 
 * You must put a space between each operand and the operator when prompted for mathematical operation.
**/
int main(int argc, char **argv)
{
	if (argc != 2 || (strcmp(argv[1], "TCP") != 0 && strcmp(argv[1], "UDP") != 0))	//Incorrect usage
	{
		printf("Usage:\t\"./client < TCP | UDP >\"\n\n");
		exit(1);
	}
	
	timeout.tv_sec = TIMEOUT;	//Set timeout value for UDP connections
	timeout.tv_usec = 0;
	
	if (getServiceAddresses() == 0)	//Connect to name server for service address then disconnect
	{
		printf("Cannot locate the service.\n\n");
		exit(2);
	}
	
	struct Message *s = (struct Message *) malloc(sizeof(struct Message));	//Message to send
	struct Message *r = (struct Message *) malloc(sizeof(struct Message));	//Message to receive
	void *packet = malloc(sizeof(struct Packet));	//Buffer for network transmission
	memset(packet, 0, sizeof(struct Packet));	//Set default values
	s->type = 0;
	s->version = 0;
	s->length = sizeof(sizeof(struct Packet));
	s->result = 0;
	s->flag = 1;
	printf("Math Lib v0.1\t\t[Type \"exit\" to quit.]\n");
	while (1)	//Main loop
	{
		if (!promptUser(s))	//Prompt user for operation
			continue;
		
		serialize(s, packet);	//Serialize request
		receivedResponse = 0;	//Set that we haven't received a response from a server yet
		for (i = 0; i < numAddresses; i++)	//Query servers in order received from name server until one responds
		{
			if (strcmp(argv[1], "TCP") == 0)	//TCP mode
			{
				if (establishConnectionTCP(i) == 0)
				{
					close(sockfd);
					continue;
				}
				send(sockfd, packet, sizeof(struct Packet), 0);	//Send request
				if (recv(sockfd, packet, sizeof(struct Packet), 0) < 1)	//Error receiving response
				{
					close(sockfd);
					continue;
				}
				else	//Response successfully received
				{
					deserialize(packet, r);	//Deserialize response
					printf("Received response: ");	//Inform user
					printResult(r);	//Print packet
					printBytes(packet);
					printMessage(r);
					close(sockfd);
					receivedResponse = 1;
					break;
				}
			}
			else	//UDP mode
			{	
				if (establishConnectionUDP(i) == 0)
				{
					close(sockfd);
					continue;
				}
				if (sendto(sockfd, packet, sizeof(struct Packet), 0, (struct sockaddr *) &server, sizeof(struct sockaddr)) < 1)	//Send request
				{
					close(sockfd);
					continue;
				}
				errno = 0;
				if (recv(sockfd, packet, sizeof(struct Packet), 0) < 1 || errno == EWOULDBLOCK)	//Error receiving response
				{
					close(sockfd);
					continue;
				}
				else	//Response successfully received
				{
					deserialize(packet, r);	//Deserialize response
					printf("Received response: ");	//Inform user
					printResult(r);	//Print packet
					printBytes(packet);
					printMessage(r);
					close(sockfd);
					receivedResponse = 1;
					break;
				}
			}
		}
		if (receivedResponse == 0)
		{
			printf("No servers are responsive.\n");
			printf("Try again later.\n\n");
		}
	}
}

/**
 * Prompts the user for input and creates a corresponding request message
 * Input: a pointer to the message to be filled
 * Returns: whether the prompted request was valid
 * Usage: > "4 * 9" + carriage_return
 * You must put a space between each operand and the operator when prompted for mathematical operation.
**/
int promptUser(struct Message *s)
{
	printf("> ");	//Prompt user
	fgets(line, 256, stdin);	//Get user input
	char *token = strtok(line, " \t");
	if (strcmp(token, "exit\n") == 0 || strcmp(token, "quit\n") == 0)	//Detect wish to exit
	{
		close(sockfd);
		exit(0);
	}
	s->operand1 = atoi(token);	//Fill message
	token = strtok(NULL, " \t");
	switch(token[0])
	{
		case 43:
			s->opcode = 1; break;
		case 45:
			s->opcode = 2; break;
		case 42:
			s->opcode = 3; break;
		case 47:
			s->opcode = 4; break;
		default:
			printf("Invalid operator.\n");
			return 0;	//Indicate invalid request
	}
	token = strtok(NULL, " \t");
	s->operand2 = atoi(token);
	return 1;
}

/**
 * Prints the result of the received message, or indicates an error if the message received had its error flag set
 * Input: the message whose result is to be displayed
 **/
void printResult(struct Message *r)
{
	if (r->flag == 1)
		printf("%lld\n", r->result);
	else
		printf("Computation error\n");
}

/**
 * Opens a connection to the default nameserver where it get the address of the service server.
 * Returns 1 upon successfully getting service addresses and 0 upon failure.
 **/
int getServiceAddresses()
{
	int namesockfd;
	struct sockaddr_in nameserver = {AF_INET, htons(DEFAULT_NAME_PORT)};
	nameserver.sin_addr.s_addr = inet_addr(DEFAULT_NAME_ADDRESS);
	
	if ((namesockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)	//Create socket
	{
		printf("Failed to create socket.\n\n");
		return 0;
	}
	if (connect(namesockfd, (struct sockaddr *) &nameserver, sizeof(struct sockaddr)) == -1)	//Connect to socket
	{
		printf("Failed to connect to nameserver.\n\n");
		return 0;
	}
	struct Service service;
	service.type = 0;	//Request service address
	service.flag = 0;
	send(namesockfd, &service, sizeof(struct Service), 0);
	if (recv(namesockfd, &service, sizeof(struct Service), 0) > 0)
	{
		numAddresses = service.numAddresses;
		for (i = 0; i < LISTSIZE; i++)
			addresses[i] = service.serviceAddress[i];
		close(namesockfd);
		return 1;
	}
	else
	{
		return 0;
		close(namesockfd);
	}
}

/**
 * Opens a UDP socket with IP address at the given index in the list.
 * Parameters: index - the index of addresses at which the IP address to be used is located.
 * Returns: 1 on success, 0 on failure
 **/
int establishConnectionTCP(int index)
{
	struct in_addr in;
	in.s_addr = addresses[index];
	server.sin_family = AF_INET;
	server.sin_port = htons(DEFAULT_SERVICE_PORT);
	server.sin_addr.s_addr = addresses[index];
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)	//Create socket
	{
		//printf("Failed to create socket.\n\n");
		return 0;
	}
	if (connect(sockfd, (struct sockaddr *) &server, sizeof(struct sockaddr)) == -1)	//Connect to socket
	{
		//printf("Failed to connect to service server.\n\n");
		return 0;
	}
	return 1;
}

/**
 * Opens a UDP socket with IP address at the given index in the list.
 * Parameters: index - the index of addresses at which the IP address to be used is located.
 * Returns: 1 on success, 0 on failure
 **/
int establishConnectionUDP(int index)
{
	struct sockaddr_in client = {AF_INET, INADDR_ANY, INADDR_ANY};
	server.sin_family = AF_INET;
	server.sin_port = htons(DEFAULT_SERVICE_PORT);
	server.sin_addr.s_addr = addresses[index];
	
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)	//Create socket
	{
		//printf("Failed to create socket.\n\n");
		return 0;
	}
	if (bind(sockfd, (struct sockaddr *) &client, sizeof(struct sockaddr)) == -1)	//Bind socket
	{
		//printf("Failed to connect to socket.\n\n");
		return 0;
	}
	
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(struct timeval));
	return 1;
}