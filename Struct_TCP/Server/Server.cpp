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
typedef struct packet {
	int num;
	int cnt;
}PAKET;

bool InitializeWindowSockets();
void CustomSelect(SOCKET s, int operation);


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

			//sad ide Receive
			iResult = recv(acceptedSocket, recvBuffer, BUFLEN, 0);

			if (iResult > 0) {
				//treba da primi strukturu
				int brojBajtova = *((int*)recvBuffer);
				printf("Primljena kolicina podataka: %d\n", brojBajtova);

				char* podaci = (char*)malloc(sizeof(brojBajtova) * sizeof(char));
				
				for (int i = 0; i < brojBajtova; i++) {
					podaci[i] = *(recvBuffer + 4 + i);
				}

				PAKET *paket = (PAKET*)podaci; //bitno kastovanje

				printf("Poslat broj: %d\nBroj paketa: %d\n\n", paket->num, paket->cnt);
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

	getchar();
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