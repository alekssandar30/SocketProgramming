// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 27016

#pragma pack(1)
typedef struct node {
	int num;
	struct node* next;
}NODE;

#pragma pack(1)
typedef struct pom_node { //same node but without pointer (this goes through net)
	int num;
}POM_NODE;

bool InitializeWindowsSockets();
void CustomSelect(SOCKET s, int operation);
void Add_on_head(NODE** list, int value);
void Add_on_tail(NODE* list, int value);
void PrintList(NODE* head);


int __cdecl main(int argc, char **argv)
{
	SOCKET connectionSocket = INVALID_SOCKET;
	int iResult;
	
	if (!InitializeWindowsSockets())
		return 1;

	connectionSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connectionSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}


	// create and initialize address structure
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddress.sin_port = htons(DEFAULT_PORT);
	// connect to server specified in serverAddress and socket connectSocket
	if (connect(connectionSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectionSocket);
		WSACleanup();
	}


	//non-blocking mode
	unsigned long nonBlockingMode = 1;
	iResult = ioctlsocket(connectionSocket, FIONBIO, &nonBlockingMode);

	if (iResult == SOCKET_ERROR)
	{
		printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
		return 1;
	}


	do {
		CustomSelect(connectionSocket, 0);

		NODE* list = NULL;
		for (int i = 1; i <= 10; i++) {
			if (i % 2 == 1)
				Add_on_head(&list, i);
			else
				Add_on_tail(list, i);
		}

		PrintList(list);

		char* buffer = (char*)malloc(10 * sizeof(int));
		int cnt = 0;

		while (list->next != NULL) {
			*((int*)buffer + cnt) = list->num;
			list = list->next;
			cnt++;
		}

		*((int*)buffer + cnt) = list->num;

		//SEND
		iResult = send(connectionSocket, buffer, 10 * sizeof(int), 0);

		free(buffer);

		printf("Bytes Sent: %ld\n", iResult);

		Sleep(1000);
	} while (1);

	getchar();

	//cleanup
	closesocket(connectionSocket);
	WSACleanup();

	return 0;
}

//*************************************************************
//					FUNCTIONS IMPLEMENTATION
//*************************************************************

bool InitializeWindowsSockets() {
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

void PrintList(NODE* head) {
	while (head != NULL) {
		printf("num: %d\n", head->num);
		head = head->next;
	}
}