#include "pch.h"
#include "ServerInfo.h"


ServerInfo::ServerInfo(SOCKET socket, int processId)
{
	connectSocket = socket;
	processID = processId;

}


ServerInfo::~ServerInfo()
{
}
