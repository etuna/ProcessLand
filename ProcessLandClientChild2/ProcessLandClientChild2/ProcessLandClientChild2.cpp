// ProcessLand.cpp : Defines the entry point for the console application.
//


#define WIN32_LEAN_AND_MEAN

#include "pch.h"
#include "ClientInfo.h"
//#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "Process.h"
#include <processthreadsapi.h>
#include <iostream>
#include <string>
#include <process.h>
#include "ServerInfo.h"

#define _GNU_SOURCE
#define SPORT "9000"
#pragma comment(lib, "Ws2_32.lib")

using namespace std;
//Process::Process(int ID, int parentID,bool isRoot, int *cPort, int *sPort)
Process *process;
_PROCESS_INFORMATION pi;

#define DEFAULT_BUFLEN 512
#define SERVER_PORT "8002"
#define CLIENT_PORT1 "7005"
#define CLIENT_PORT2 "7006"

int quit = 0;
int *notification;
int *processes;
int processCounter = 0;
int messageRecieved1 = 0;
int messageRecieved2 = 0;
int isLeaf = 1;
string line;
void sendNotification();
unsigned __stdcall  consoleThread(void*);
unsigned __stdcall  clientSession(void* client);
unsigned __stdcall  serverSession(void* server);
void signalChildThreads(char[]);
int clients[2];
int main()
{
	cout << "LEAF2 (child of client1) started." << endl;
	cout << "------------------------------------------------------" << endl;
	int run = 1;

	//CLIENT PROCESS
	int *sport;
	int sp[] = { 7001, 7002 }; // Server for clients
	sport = sp;

	int *cport;
	int cp[] = { 8001 }; //Client for Server
	cport = cp;

	int pid = GetCurrentProcessId();
	process = new Process(pid, 0, true, cport, sport);

	//Client for Server
	WSADATA wsaDataClient;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	//char *sendbuf = "this is a test";
	char recvbuf[DEFAULT_BUFLEN];
	int iResultClient;
	int recvbuflen = DEFAULT_BUFLEN;


	// Initialize Winsock
	iResultClient = WSAStartup(MAKEWORD(2, 2), &wsaDataClient);
	if (iResultClient != 0) {
		printf("WSAStartup failed with error: %d\n", iResultClient);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResultClient = getaddrinfo("localhost", SERVER_PORT, &hints, &result);
	if (iResultClient != 0) {
		printf("getaddrinfo failed with error: %d\n", iResultClient);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResultClient = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResultClient == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}
	cout << "LEAF2:  Connection has been established with server" << endl;
	cout << "------------------------------------------------------" << endl;
	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}


	unsigned clientThreadID;
	ServerInfo *server = new ServerInfo(ConnectSocket, pid);
	HANDLE ClientThread = (HANDLE)_beginthreadex(NULL, 0, &serverSession, (void*)server, 0, &clientThreadID);



	//---------------------------------

	//Console Thread
	unsigned consoleThreadID;
	HANDLE ConsoleThread = (HANDLE)_beginthreadex(NULL, 0, &consoleThread, NULL, 0, &consoleThreadID);
	//-------------------


	//Child Thread----------------

	WSADATA wsaDataServer;
	int iResultServer;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo *cresult = NULL;
	struct addrinfo chints;

	int iSendResultServer;
	char crecvbuf[DEFAULT_BUFLEN];
	int crecvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
	iResultServer = WSAStartup(MAKEWORD(2, 2), &wsaDataServer);
	if (iResultServer != 0) {
		printf("WSAStartup failed with error: %d\n", iResultServer);
		return 1;
	}

	ZeroMemory(&chints, sizeof(chints));
	chints.ai_family = AF_INET;
	chints.ai_socktype = SOCK_STREAM;
	chints.ai_protocol = IPPROTO_TCP;
	chints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResultServer = getaddrinfo(NULL, CLIENT_PORT1, &chints, &cresult);
	if (iResultServer != 0) {
		printf("getaddrinfo failed with error: %d\n", iResultServer);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(cresult->ai_family, cresult->ai_socktype, cresult->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(cresult);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket
	iResultServer = bind(ListenSocket, cresult->ai_addr, (int)cresult->ai_addrlen);
	if (iResultServer == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(cresult);

	iResultServer = listen(ListenSocket, SOMAXCONN);
	if (iResultServer == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}


	int clientCounter = 0;
	while (run == 1) {

		while ((ClientSocket = accept(ListenSocket, NULL, NULL))) {
			unsigned clientThreadID;
			ClientInfo *client = new ClientInfo(ClientSocket, processCounter, clientCounter);
			HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &clientSession, (void*)client, 0, &clientThreadID);
			processes[processCounter] = 0;
			processCounter++;
			clientCounter++;
			cout << "CLIENT CHILD : A Child process connection has been established" << endl;
		}
	}
	return 0;

}

//Handle client session with a thread for each
unsigned __stdcall  clientSession(void* client) {
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	ClientInfo *mClient = (ClientInfo*)client;
	int ClientID = mClient->process;
	SOCKET ClientSocket = (SOCKET)(mClient->clientSocket);
	int process = mClient->process;
	while (quit == 0) {
		/*
				while (notification[process] == 0) {
					Sleep(1);
				}
				if (notification[process] == 1) {
					notification[process] = 0;
				}*/

				//cout << "ROOT: " << line << endl;

		if (clients[ClientID] == 1) {

			// Echo the buffer back to the sender
			string tempLineC = "test";
			int n = line.length();
			char* charArr = new char[n + 1];
			strcpy(charArr, line.c_str());
			int iSendResult = send(ClientSocket, charArr, n + 1, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
			//printf("ROOT : Message %s sent. Bytes sent: %d\n", line, iSendResult);
			clients[ClientID] = 0;
		}
	}


	return 0;
}

unsigned __stdcall  serverSession(void* server) {
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	ServerInfo *mServer = (ServerInfo*)server;
	SOCKET ConnectSocket = (SOCKET)(mServer->connectSocket);
	int process = mServer->processID;
	int iResult;

	while (quit == 0) {

		// Receive until the peer closes the connection
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			//printf("Bytes received: %d\n", iResult);
			cout << "LEAF2: Message recieved : " << recvbuf << endl;
			cout << "------------------------------------------------------" << endl;
			signalChildThreads(recvbuf);
		}

	}



	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}

unsigned __stdcall  consoleThread(void*) {
	
	while (quit == 0) {

		getline(cin, line);
		if (isLeaf == 1) {
			cout << "LEAF2:I am leaf" << endl;

		}
		else {
			cout << "Line read: " << line << endl;
			if ((strcmp(line.c_str(), "quit") == 0)) {
				quit = 1;
				exit(1);
			}

			sendNotification();
		}

	}
}

void sendNotification() {
	int size = sizeof(*notification);

	int i = 0;
	for (i = 0; i < size; i++) {
		notification[i] = 1;
	}

}

void signalChildThreads(char arr[]) {
	line = arr;

	clients[0] = 1;
	clients[1] = 1;
}