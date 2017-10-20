// chat_client.cpp// from (c) https://msdn.microsoft.com/en-us/library/windows/desktop/ms737591(v=vs.85).aspx

//////////////////////////////////////////	Declarations, Includes, Globals, Structs:
#include "stdafx.h"
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "client_funcs.h"

int iResult = 0;

int my_init_socket();
int my_connection(const char* host, const char* port, SOCKET* clientSocket);
int my_send_message(SOCKET clientSocket, char* sendbuf, size_t theLenth, int theZero);
int my_recv_message(SOCKET clientSocket, char* recvbuf, int recvbuflen);
int my_shutdown(SOCKET clientSocket, int how);
void my_cleanup(SOCKET clientSocket);
void check_result(int iResult, char* funcName);  // Usage Func

struct addrinfo *result = NULL;
struct addrinfo *ptr = NULL;
struct addrinfo hints;

//////////////////////////////////////////	FUNCTIONS:
int my_init_socket()
{
	WSADATA wsaData;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1; // Error 
	}

	return 0;
}

int my_connection(const char* host, const char* port, SOCKET* clientSocket)
{
	int initResult = my_init_socket();
	check_result(initResult, "initSocketStartup");

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(host, port, &hints, &result); // e.g 192.168.1.104", DEFAULT_PORT,...
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	if (result == NULL) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	//SOCKET clientSocket = INVALID_SOCKET;
	*clientSocket = INVALID_SOCKET;
	*clientSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (*clientSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Connect to server.
	iResult = connect(*clientSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("Unable to connect to server!\n");
		closesocket(*clientSocket);
		WSACleanup();
		return 1;
	}

	return iResult;
}

int my_send_message(SOCKET clientSocket, char* sendbuf, size_t theLenth, int theZero)
{
	iResult = send(clientSocket, sendbuf, strlen(sendbuf), 0);
	
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}
	
	return iResult;
}

//void recv_message(SOCKET s, char[] buffer, int buffer_max_len) 
int my_recv_message(SOCKET clientSocket, char* recvbuf, int recvbuflen)
{
	// Receive until the peer closes the connection
	do {

		iResult = recv(clientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
			printf("Bytes received: %d\n", iResult);
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed with error: %d\n", WSAGetLastError());

	} while (iResult > 0);
	
	return iResult;
}

int my_shutdown(SOCKET clientSocket, int how) // how == SD_SEND
{
	// shutdown the connection since no more data will be sent
	iResult = shutdown(clientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}

	return iResult;
}

void my_cleanup(SOCKET clientSocket)
{
	// cleanup
	closesocket(clientSocket);
	WSACleanup();
}

void check_result(int iResult, char* funcName)  // Usage Func
{
	if (iResult == 0)
		printf("check result: '%s'\t compeleted.\n", funcName);

	else if (iResult == 1)
		printf("check result: '%s'\t stoped!\n", funcName);

	else
		printf("check result: '%s'\t stoped by iResult: %d!\n", funcName, iResult);
}

//////////////////////////////////////////	Main:
int main(int argc, char **argv)
{
	SOCKET clientSocket = INVALID_SOCKET;
	const char* myHost = "127.0.0.1";
	const char* myPort = DEFAULT_PORT;

	// creating the socket and connecting
	iResult = my_connection(myHost, myPort, &clientSocket); // e.g connect_socket("192...", 15000); for passing IP and Port. 
	check_result(iResult, "my_connection");
	
	freeaddrinfo(result);

	/// preparing the send_message parameters:
	char *sendbuf = "Agha kojaei?";	// e.g. a  char* message
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	///
	// Send an initial buffer
	iResult = my_send_message(clientSocket, sendbuf, (size_t)strlen(sendbuf), 0); // e.g. send(clientSocket, "Agha...");
	printf("Bytes Sent: %ld\n", iResult);
	//check_result(iResult, "Sending");

	iResult = 0;
	iResult = my_recv_message(clientSocket, recvbuf, recvbuflen); // e.g. recv(client, buffer, 1000);
	check_result(iResult, "Receiving");

	// shutdown the connection since no more data will be sent
	iResult = my_shutdown(clientSocket, SD_SEND);
	check_result(iResult, "Shutdown");

	// cleanup
	my_cleanup(clientSocket);

	return 0;
}
