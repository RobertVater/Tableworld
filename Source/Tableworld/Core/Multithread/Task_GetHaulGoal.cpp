// Copyright by Robert Vater (Gunschlinger)

#include "Task_GetHaulGoal.h"
#include "World/Tile/Building/ProductionBuilding.h"
#include "World/Tile/TileData.h"
#include "World/Tile/Building/BuildableTile.h"
#include "World/Tile/Building/CityCentreBuilding.h"

Task_GetHaulGoal::Task_GetHaulGoal(ATableWorldTable* nWorldTable, ACityCentreTile* nCityCenter)
{
	WorldTable = nWorldTable;
	HomeTile = nCityCenter;
}

Task_GetHaulGoal::~Task_GetHaulGoal()
{

}

void Task_GetHaulGoal::DoWork()
{
	FPlatformProcess::Sleep(0.03);

	if(HomeTile)
	{
		if(WorldTable)
		{
			FVector2D StartTile;
			FVector2D EndTile;
			AProductionBuilding* FoundBuilding = HomeTile->getValidHaulGoal(StartTile, EndTile);

			if(FoundBuilding)
			{
				//Give the hometile its building
				HomeTile->SetValidHaulGoal(FoundBuilding, StartTile, EndTile);
			}
		}
	}
}
