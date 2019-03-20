// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class TABLEWORLD_API Thread_Test : public FNonAbandonableTask
{
public:
	Thread_Test();
	~Thread_Test();

	FORCEINLINE TStatId GetStatId() const 
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(Thread_Test, STATGROUP_ThreadPoolAsyncTasks);
	}

	int32 LastNumber = 0;

	void DoWork();

	void DoWorkMain();
};
