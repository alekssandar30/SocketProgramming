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

/*
Slanje strukture preko mreze
*/

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 27016

#pragma pack(1)
typedef struct packet {
	int num; 
	int cnt;
}PAKET;

bool InitializeWindowsSockets();
void CustomSelect(SOCKET s, int operation);
void CustomSend(SOCKET s, char* niz, int* brojBajtova);


int __cdecl main(int argc, char **argv)
{

	SOCKET connectionSocket = INVALID_SOCKET;
	int iResult;
	//PAKET* paket = (PAKET*)malloc(sizeof(PAKET));
	char sendBuff[sizeof(PAKET)];
	int counter = 1;
	char poruka[DEFAULT_BUFLEN];
	

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

	int broj;

	while (1) {
		printf("Unesi broj za slanje: ");
		scanf("%d", &broj);

		//napuni strukturu i prepakuj strukturu u niz?
		PAKET paket;
		paket.num = broj;
		paket.cnt = counter++;

		char* buff = (char*)malloc(4 + sizeof(PAKET));
		*((int*)buff) = sizeof(PAKET);
		*((int*)buff + 1) = paket.num;
		*((int*)buff + 2) = paket.cnt;

		CustomSelect(connectionSocket, 0);

		//sad ide Send
		int brojBajtova = *(int*)buff;
		iResult = send(connectionSocket, buff, 4 + sizeof(PAKET), 0);

		printf("Poslata kolicina podataka: %d\n\n", brojBajtova);

		free(buff);
	}

	
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

//void CustomSend(SOCKET s, char* niz, int* brojBajtova) {
//	int iResult = 0;
//	int bytesSent = 0;
//	int bytesToSend = *brojBajtova;
//
//	CustomSelect(s, 0);
//
//	iResult = send(s, (char*)&brojBajtova, 4, NULL);
//
//	do {
//		CustomSelect(s, 0);
//
//		iResult = send(s, niz + bytesSent, *brojBajtova - bytesSent, NULL);
//
//		if (iResult == 0) {
//			continue;
//		}
//		else if (iResult == -1) {
//			printf("\nGreska\n");
//			return;
//		}
//
//		bytesSent += iResult;
//	} while (bytesSent < *brojBajtova);
//
//	printf("Uspesno poslato\n");
//	getchar();
//
//	return;
//}