// Copyright by Robert Vater (Gunschlinger)

#include "HarvesterTile.h"
#include "World/TableWorldTable.h"
#include "Creature/HarvesterCreature.h"
#include "../TileData.h"
#include "Core/TableGamemode.h"

void AHarvesterTile::Place(TArray<FVector2D> nPlacedOnTiles, FTableBuilding nBuildingData)
{
	Super::Place(nPlacedOnTiles, nBuildingData);

	StartWork();
}

void AHarvesterTile::StartWork()
{
	Super::StartWork();

	//Try to find valid rescource tiles
	if (getTable())
	{
		HarvestAbleTiles = getTable()->getRescourcesInRadius(getTileX(),getTileY(), SearchRangeInTiles + (int32)(BuildingData.BuildingSize.X + BuildingData.BuildingSize.Y), HarvestRescource );

		for (int32 i = 0; i < MaxHarvesterCreatureCount; i++) 
		{
			if (HarvestAbleTiles.Num() > 0)
			{
				TryCreateCreature();
			}
		}
	}
}

void AHarvesterTile::TryCreateCreature()
{
	//Check if we have a creature slot avaiable
	if(Workers.Num() < MaxHarvesterCreatureCount)
	{
		if (HarvestAbleTiles.Num() > 0) 
		{
			AHarvesterCreature* NewWorker = SpawnCreature();
			if (NewWorker)
			{
				Workers.Add(NewWorker);
			}
		}
	}
}

AHarvesterCreature* AHarvesterTile::SpawnCreature()
{
	UTileData* SpawnTile = getValidTile();
	if(SpawnTile)
	{
		UTileData* HarvestTile = getNextHarvestTile();
		if (HarvestTile)
		{
			AHarvesterCreature* Harvester = GetWorld()->SpawnActor<AHarvesterCreature>(WorkerClass, SpawnTile->getWorldCenter(), FRotator::ZeroRotator);
			if (Harvester)
			{
				HarvestTile->GiveHarvester();
				
				Harvester->Create(FVector2D(SpawnTile->getX(), SpawnTile->getY()), this);
				Harvester->GiveHarvestJob(HarvestTile);

				return Harvester;
			}
		}
	}

	return nullptr;
}

void AHarvesterTile::OnWorkerReturn(AHarvesterCreature* Creature)
{
	if(Creature)
	{
		UpdateHarvestableTiles();

		if(!bIsWorking)
		{
			//This building isnt active. Destroy the worker
			Creature->Destroy();
			return;
		}

		if(Creature->hasHarvested())
		{
			if (CurrentInventory < InventorySize)
			{
				//Add a item to the inventory
				CurrentInventory++;

				if(getGamemode())
				{
					getGamemode()->AddFloatingItem(ProducedItems, 1, GetActorLocation());
				}

			}else
			{
				Creature->Destroy();
				Workers.Empty();
				
				//If the inventory is full stop working
				StopWork();
				return;
			}

			Creature->ResetHasHarvested();
		}

		//If the worker has no valid tile anymore try to give it a new one
		if(!Creature->HasTileStillRescources())
		{
			UTileData* NextTile = getNextHarvestTile();
			if(NextTile)
			{
				//Mark the tile as taken
				NextTile->GiveHarvester();

				//Give the worker the harvest job
				Creature->GiveHarvestJob(NextTile);
				return;
			}else
			{
				//We have no tiles left. Destroy the worker
				Workers.Remove(Creature);
				Creature->Destroy();
				return;
			}
		}

		DebugWarning("Return Worker");

		//Just let the creature return to his rescource
		Creature->GiveHarvestJob(Creature->getHarvestTile());
	}
}

void AHarvesterTile::UpdateHarvestableTiles()
{
	for (int32 i = 0; i < HarvestAbleTiles.Num(); i++)
	{
		UTileData* Tile = HarvestAbleTiles[i];
		if (Tile)
		{
			if(!Tile->HasRescource() || Tile->HasHarvester())
			{
				HarvestAbleTiles.RemoveAt(i);
			}
		}
	}
}

UTileData* AHarvesterTile::getNextHarvestTile()
{
	for(int32 i = 0; i < HarvestAbleTiles.Num(); i++)
	{
		UTileData* Tile = HarvestAbleTiles[i];
		if(Tile)
		{
			if(!Tile->HasHarvester())
			{
				return Tile;
			}
		}
	}

	return nullptr;
}

int32 AHarvesterTile::getBuildGridRadius()
{
	if(BuildingData.ID != NAME_None)
	{
		return SearchRangeInTiles + (int32)(BuildingData.BuildingSize.X + BuildingData.BuildingSize.Y);
	}

	return Super::getBuildGridRadius();
}
