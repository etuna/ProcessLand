#include "pch.h"
#include "Process.h"


Process::Process(int ID, int parentID, bool isRoot, int *cPort, int *sPort)
{
	confProcess(ID, parentID, isRoot, cPort, sPort);
}


Process::~Process()
{

}


bool Process::confProcess(int id, int parentid, bool isroot, int *cport, int *sport) {
	ID = id;
	parentID = parentid;
	isRoot = isroot;
	cPort = cport;
	sPort = sport;
	return true;
}