#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <vector>

using namespace std;
class Process
{

public:
	Process(int, int, bool, int*, int*);
	~Process();
	bool isRoot;
	int getID();
	int* sPort;
	int *cPort;
	bool confProcess(int, int, bool, int*, int*);
	Process *parent;


private:
	int ID;
	int parentID;
};

