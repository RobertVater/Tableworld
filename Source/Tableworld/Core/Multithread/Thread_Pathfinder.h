// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "Misc/TableHelper.h"
#include "World/TableWorldTable.h"

class UTileData;


class Task_Pathfinder : public FNonAbandonableTask
{
public:

	Task_Pathfinder(ATableWorldTable* nWorldTable, FTablePathfindingRequest Request)
	{
		WorldTable = nWorldTable;
		OpenRequest = Request;
		DebugWarning("-PathfinderTask Started-");
	}

	~Task_Pathfinder()
	{
		DebugWarning("-PathfinderTask Ended-");
	}

	void DoWork()
	{
		FPlatformProcess::Sleep(0.03);
		
		if(WorldTable)
		{
			TArray<UTileData*> Path = WorldTable->FindPath(OpenRequest);
			DebugLog("Found a Path " + FString::FromInt(Path.Num()));
		}
	}

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(Thread_Test, STATGROUP_ThreadPoolAsyncTasks);
	}

	bool isThreadWorking()
	{
		return bIsWorking;
	}

	bool bThreadActive = true;
	bool bIsWorking = false;

	ATableWorldTable* WorldTable = nullptr;
	FTablePathfindingRequest OpenRequest;

};


class TABLEWORLD_API Thread_Pathfinder
{
public:
	Thread_Pathfinder(ATableWorldTable* nWorldTable);
	~Thread_Pathfinder();

	void AddPathRequest(FTablePathfindingRequest Request);

	TArray<Task_Pathfinder*> TaskList;

protected:

	ATableWorldTable* WorldTable = nullptr;
};