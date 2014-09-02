#include <stdint.h>
#include <arpa/inet.h>

#ifndef LISTSIZE	//Too lazy to write lists, so I use arrays of this fixed size
  #define LISTSIZE 4
#endif
#ifndef TIMEOUT
  #define TIMEOUT 2
 #endif
#ifndef DEFAULT_NAME_ADDRESS
  #define DEFAULT_NAME_ADDRESS "129.120.151.97"	//Address of cse04.cse.unt.edu
#endif
#ifndef DEFAULT_NAME_PORT
  #define DEFAULT_NAME_PORT 12346
#endif
#ifndef DEFAULT_SERVICE_PORT
  #define DEFAULT_SERVICE_PORT 12345
#endif

/**
 * Author: James Spiares
 * CSCE 3530 Project 1 Solution
 * Header file for message structure and network-byte-order conversion functions
 **/
 
struct Message	//The structure which holds a message
{
	unsigned char type;
	unsigned char version;
	unsigned char length;
	unsigned int operand1;
	unsigned int operand2;
	unsigned char opcode;
	unsigned char flag;
	unsigned long long result;
};

#pragma pack(1)	//Pack without alignment to meet message standard
struct Packet	//The structure which holds a network-ready message
{
	uint8_t type;
	uint8_t version;
	uint8_t length;
	uint32_t operand1;
	uint32_t operand2;
	uint8_t codeflag;
	uint64_t result;
};

#pragma pack(1)	//Pack witout alignment
struct Service	//The structure which holds a network-ready name service request
{
	uint8_t type;
	uint8_t flag;
	uint8_t numAddresses;
	uint32_t serviceAddress[LISTSIZE];
};

/**
 * See <network.c> for documentation of these functions.
 **/
void serialize(struct Message *, struct Packet *);
void deserialize(struct Packet *, struct Message *);

void printBytes(struct Packet *);
void printMessage(struct Message *);
