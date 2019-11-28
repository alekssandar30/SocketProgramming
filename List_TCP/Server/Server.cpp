// Server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#pragma once

#include "pch.h"
#include <iostream>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFLEN 512
#define PORT "27016"

#pragma pack(1)
typedef struct node {
	int num;
	struct node* next;
}NODE;

#pragma pack(1)
typedef struct pom_node { //same node but without pointer (this goes through net)
	int num;
}POM_NODE;

bool InitializeWindowSockets();
void CustomSelect(SOCKET s, int operation);
void PrintList(NODE* head);
void Add_on_head(NODE** list, int value);
void Add_on_tail(NODE* list, int value);


int main()
{
	SOCKET listenSocket = INVALID_SOCKET;
	SOCKET acceptedSocket = INVALID_SOCKET;

	int iResult;
	char recvBuffer[BUFLEN];

	if (!InitializeWindowSockets())
		return 1;

	// Prepare address information structures
	addrinfo *resultingAddress = NULL;
	addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, PORT, &hints, &resultingAddress);
	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	//Create Socket
	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		WSACleanup();
		return 1;
	}


	//Bind
	iResult = bind(listenSocket, resultingAddress->ai_addr,
		(int)resultingAddress->ai_addrlen);

	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(resultingAddress);


	//Listen
	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	printf("Server initialized, waiting for clients.\n");
	

	do {
		// Wait for clients and accept client connections.
        // Returning value is acceptedSocket used for further
        // Client<->Server communication. This version of
        // server will handle only one client.
		// Initialize select parameters

		CustomSelect(listenSocket, 1);

		acceptedSocket = accept(listenSocket, NULL, NULL);

		if (acceptedSocket == INVALID_SOCKET) {
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}

		do {
			//sad ce opet ici select za acceptedSocket
			CustomSelect(acceptedSocket, 1);

			iResult = recv(acceptedSocket, recvBuffer, BUFLEN, 0);

			if (iResult > 0) {
				NODE* list = NULL;
				for (int i = 1; i <= 10; i++) {
					if (i == 1) {
						Add_on_head(&list, *((int*)recvBuffer + (i - 1)) );
					}
					else {
						Add_on_tail(list, *((int*)recvBuffer + (i - 1)));
					}
				}

				PrintList(list);
			}
			else if (iResult == 0) {
				printf("Connection with client closed.\n");
				closesocket(acceptedSocket);
			}
			else {
				printf("recv failed with error: %d\n", WSAGetLastError());
				closesocket(acceptedSocket);
			}

			

		} while (iResult > 0);

	} while (1);

	//shutdown
	iResult = shutdown(acceptedSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(acceptedSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(listenSocket);
	closesocket(acceptedSocket);
	WSACleanup();

	return 0;
}


//*************************************************************
//					FUNCTIONS IMPLEMENTATION
//*************************************************************


bool InitializeWindowSockets() {
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return false;
	}
	
	return true;
}


void CustomSelect(SOCKET s, int operation) { //operation -> 1->read, 0 -> write
	if (operation) {
		//citanje
		do {
			FD_SET readSet;
			timeval timeVal;

			FD_ZERO(&readSet);
			FD_SET(s, &readSet);

			timeVal.tv_sec = 1;
			timeVal.tv_usec = 0;

			int iResult = select(0, &readSet, NULL, NULL, &timeVal);

			if (iResult > 0) {
				break;
			}


		} while (1);
	}
	else {
		//pisanje
		do {
			FD_SET writeSet;
			timeval timeVal;

			FD_ZERO(&writeSet);
			FD_SET(s, &writeSet);

			timeVal.tv_sec = 1;
			timeVal.tv_usec = 0;

			int iResult = select(0, NULL, &writeSet, NULL, &timeVal);

			if (iResult > 0) {
				break;
			}

		} while (1);
	}
}

void PrintList(NODE* head) {
	while (head != NULL) {
		printf("num: %d\n", head->num);
		head = head->next;
	}
}

void Add_on_head(NODE** list, int value) {
	NODE* newNode = (NODE*)malloc(sizeof(NODE));
	if (newNode == NULL)
		return;

	newNode->num = value;
	newNode->next = *list;

	*list = newNode;
}

void Add_on_tail(NODE* list, int value) {
	while (list->next != NULL) {
		list = list->next;
	}

	NODE* newNode = (NODE*)malloc(sizeof(NODE));
	if (newNode == NULL)
		return;

	newNode->next = NULL;
	newNode->num = value;

	list->next = newNode;

}