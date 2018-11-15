#include "pch.h"
#include "ClientInfo.h"


ClientInfo::ClientInfo(SOCKET sock, int processId, int Clientid)
{
	clientSocket = sock;
	process = processId;
	clientID = Clientid;
}


ClientInfo::~ClientInfo()
{
}
