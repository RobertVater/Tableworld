// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"

class ATableWorldTable;
class ACityCentreTile;

class TABLEWORLD_API Task_GetHaulGoal : public FNonAbandonableTask
{
public:
	Task_GetHaulGoal(ATableWorldTable* nWorldTable, ACityCentreTile* CityCenter);
	~Task_GetHaulGoal();

	void DoWork();

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(Thread_Test, STATGROUP_ThreadPoolAsyncTasks);
	}

protected:

	ATableWorldTable* WorldTable = nullptr;
	ACityCentreTile* HomeTile;
};
