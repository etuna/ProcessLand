#pragma once
#include <winsock2.h>
class ServerInfo
{
public:
	ServerInfo(SOCKET, int);
	~ServerInfo();
	SOCKET connectSocket;
	int processID;
};

