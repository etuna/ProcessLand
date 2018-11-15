#pragma once
#include <winsock2.h>
class ClientInfo
{
public:
	ClientInfo(SOCKET, int,int);
	~ClientInfo();
	SOCKET clientSocket;
	int process;
	int clientID;
};

