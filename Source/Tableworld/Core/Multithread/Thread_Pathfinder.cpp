// Copyright by Robert Vater (Gunschlinger)

#include "Thread_Pathfinder.h"
#include "World/TableWorldTable.h"
#include "Misc/TableHelper.h"

Thread_Pathfinder::Thread_Pathfinder(ATableWorldTable* nWorldTable)
{
	WorldTable = nWorldTable;
}

Thread_Pathfinder::~Thread_Pathfinder()
{
	
}

void Thread_Pathfinder::AddPathRequest(FTablePathfindingRequest Request)
{
	(new FAutoDeleteAsyncTask<Task_Pathfinder>(WorldTable, Request))->StartBackgroundTask();
}