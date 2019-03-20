// Copyright by Robert Vater (Gunschlinger)

#include "Thread_Test.h"

Thread_Test::Thread_Test()
{
	DebugError("--Task Started--");
}

Thread_Test::~Thread_Test()
{
	DebugError("--Task Finished--");
}

void Thread_Test::DoWork()
{
	int32 found = 0;
	int32 test = 2;
	
	while(LastNumber < 9999999)
	{
		bool bPrime = true;

		for(int32 i = 2; i < test / 2; i++)
		{
			if(test % i == 0)
			{
				bPrime = false;
				break;
			}
		}

		if(bPrime)
		{
			LastNumber++;
			if(LastNumber % 1000 == 0)
			{
				//DebugWarning("FOund " + FString::FromInt(LastNumber));
			}
		}

		test++;
	}
}

void Thread_Test::DoWorkMain()
{
	DoWork();
}
