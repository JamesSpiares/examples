/**
 * Author: James Spiares
 * CSCE 3530 Project 1 Solution
 * Implementation for network-byte-order conversion functions and message structure definition
 **/

#include <string.h>
#include <stdio.h>
#include <endian.h>
#include "network.h"

/**
 * Serializes the message m into a network-ready format
 * Input:	m - the message to be serialized
 * 			p - buffer for the network-ready message
 **/
void serialize(struct Message *m, struct Packet *p)
{
	uint32_t op1 = htobe32(m->operand1);	//Convert to big endian
	uint32_t op2 = htobe32(m->operand2);
	uint64_t res = htobe64(m->result);
	uint8_t	code = (m->opcode << 1) + m->flag;	//Combine opcode and flag
	
	memcpy(&p->type, &m->type, 1);	//Pack
	memcpy(&p->version, &m->version, 1);
	memcpy(&p->length, &m->length, 1);
	memcpy(&p->operand1, &op1, 4);
	memcpy(&p->operand2, &op2, 4);
	memcpy(&p->codeflag, &code, 1);
	memcpy(&p->result, &res, 8);
}

/**
 * Deserializes the received network data in p into a manipulable message m
 * Input:	p - the network data to be deserialized
 * 			m - the message to be filled
 **/
void deserialize(struct Packet *p, struct Message *m)
{
	unsigned int op1 = be32toh(p->operand1);	//Convert to host endian
	unsigned int op2 = be32toh(p->operand2);
	unsigned long long res = be64toh(p->result);
	unsigned char code = p->codeflag >> 1;	//Separate opcode and flag
	unsigned char flag = p->codeflag & 1;
	
	memcpy(&m->type, &p->type, 1);	//Unpack
	memcpy(&m->version, &p->version, 1);
	memcpy(&m->length, &p->length, 1);
	memcpy(&m->operand1, &op1, 4);
	memcpy(&m->operand2, &op2, 4);
	memcpy(&m->opcode, &code, 1);
	memcpy(&m->flag, &flag, 1);
	memcpy(&m->result, &res, 8);
}

/**
 * Prints the bytes in a packet.
 **/
void printBytes(struct Packet *p)
{
	char *q = (char *) p;
	int i;
	for (i = 0; i < 20; i++)
		printf("%.2hhX ", *(q + i));
	printf("\n");
}

/**
 * Prints the members in a struct Message.
 **/
void printMessage(struct Message *m)
{
	printf("Type: %u\n", m->type);
	printf("Version: %u\n", m->version);
	printf("Length: %u\n", m->length);
	printf("Operand 1: %u\n", m->operand1);
	printf("Operand 2: %u\n", m->operand2);
	printf("Opcode: %u\n", m->opcode);
	printf("Flag: %u\n", m->flag);
	printf("Result: %lld\n", m->result);
	printf("\n");
}
