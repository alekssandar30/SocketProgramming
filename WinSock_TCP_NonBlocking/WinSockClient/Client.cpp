#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 27016

// Initializes WinSock2 library
// Returns true if succeeded, false otherwise.
bool InitializeWindowsSockets();
int CustomSelect(SOCKET s, int operation); ///operation -> 1=read, 0->write

int __cdecl main(int argc, char **argv)
{
	// socket used to communicate with server
	SOCKET connectSocket = INVALID_SOCKET;
	int iResult;
	char *messageToSend = "this is a test";

	if (InitializeWindowsSockets() == false)
	{
		return 1;
	}

	// create a socket
	connectSocket = socket(AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP);

	if (connectSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// create and initialize address structure
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddress.sin_port = htons(DEFAULT_PORT);

	if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
	}

	//Set NonBlocking mode
	unsigned long nonBlockingMode = 1;
	iResult = ioctlsocket(connectSocket, FIONBIO, &nonBlockingMode);

	if (iResult == SOCKET_ERROR)
	{
		printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
		return 1;
	}

	while (1) {

		//select
		iResult = CustomSelect(connectSocket, 0);

		if (iResult == SOCKET_ERROR)
		{
			fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
			continue;
		}

		// Send an prepared message with null terminator included

		if (iResult > 0) {
			iResult = send(connectSocket, messageToSend, (int)strlen(messageToSend) + 1, 0);
		}
		else if (iResult == 0) {
			printf("Vreme cekanja isteklo.\n");
			Sleep(500);
		}
		else {
			printf("Greska\n");
		}
		

		if (iResult == SOCKET_ERROR)
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(connectSocket);
			WSACleanup();
			return 1;
		}

		printf("Bytes Sent: %ld\n", iResult);
	}


	getchar();
	// cleanup
	closesocket(connectSocket);
	WSACleanup();

	return 0;
}

bool InitializeWindowsSockets()
{
	WSADATA wsaData;
	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}

int CustomSelect(SOCKET s, int operation) {
	int iResult;
	if (operation) { //1
		do {

			FD_SET readSet;
			timeval timeVal;

			FD_ZERO(&readSet);
			FD_SET(s, &readSet);

			timeVal.tv_sec = 1;
			timeVal.tv_usec = 0;

			iResult = select(0, &readSet, NULL, NULL, &timeVal);

			if (iResult > 0) {
				return iResult;
			}
			

		} while (1);

	}
	else {
		do {

			FD_SET writeSet;
			timeval timeVal;

			FD_ZERO(&writeSet);
			FD_SET(s, &writeSet);

			timeVal.tv_sec = 1;
			timeVal.tv_usec = 0;

			iResult = select(0, NULL, &writeSet, NULL, &timeVal);

			if (iResult > 0)
				return iResult;

		} while (1);


	}

	return 0;
}