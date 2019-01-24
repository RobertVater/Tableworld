// Copyright by Robert Vater (Gunschlinger)

#include "HarvesterTile.h"
#include "World/TableWorldTable.h"
#include "Creature/HarvesterCreature.h"
#include "../TileData.h"
#include "Core/TableGamemode.h"
#include "Core/TableGameInstance.h"

void AHarvesterTile::Place(TArray<FVector2D> nPlacedOnTiles, FTableBuilding nBuildingData)
{
	Super::Place(nPlacedOnTiles, nBuildingData);

	if (getTable())
	{
		//Get the tiles we are allowed  to harvest
		switch (HarvesterType)
		{
		case EHarvesterType::Rescource:
			HarvestAbleTiles = getTable()->getRescourcesInRadius(getTileX(), getTileY(), getBuildGridRadius(), HarvestRescource);
			break;

		case EHarvesterType::Tile:
			HarvestAbleTiles = getTable()->getTilesInRadius(getTileX(), getTileY(), getBuildGridRadius(), HarvestTile, true);
			break;
		}

		StartWork();
	}
}

void AHarvesterTile::StartWork()
{
	Super::StartWork();

	InitWorkers();
}

void AHarvesterTile::StopWork()
{
	Super::StopWork();

	//Move all workers back home. No matter what they do
	for(int32 i = 0; i < Workers.Num(); i++)
	{
		AHarvesterCreature* Worker = Workers[i];
		if(Worker)
		{
			if(Worker->getStatus() != ECreatureStatus::Deactivated)
			{
				//Clear the worker tile
				if(Worker->getHarvestTile())
				{
					Worker->getHarvestTile()->ClearHarvester();
				}
				
				//Return all workers
				Worker->GiveReturnJob();
			}
		}
	}
}

void AHarvesterTile::InitWorkers()
{
	//Check if we need to spawn new workers.
	if(Workers.Num() < MaxWorkerCount)
	{
		int32 NeededWorkers = MaxWorkerCount - Workers.Num();
		DebugWarning("Spawning " + FString::FromInt(NeededWorkers) + " workers!");
		for(int32 i = 0; i < NeededWorkers; i++)
		{
			AHarvesterCreature* NewWorker = SpawnWorker();
			if(NewWorker)
			{
				Workers.Add(NewWorker);
			}
		}
	}

	//Send all workers to work
	for(int32 i = 0; i < Workers.Num(); i++)
	{
		AHarvesterCreature* Worker = Workers[i];
		if(Worker)
		{
			//Get a suitable tile for the worker
			UTileData* NewHarvestTile = getNextHarvestTile();
			if(NewHarvestTile == nullptr)
			{
				//Deactivate this worker.
				DeactivateWorker(Worker);
				continue;
			}

			//Activate workers
			ActivateWorker(Worker);

			NewHarvestTile->GiveHarvester();
			Worker->GiveHarvestJob(NewHarvestTile);
		}
	}
}

AHarvesterCreature* AHarvesterTile::SpawnWorker()
{
	if (WorkerClass)
	{
		FVector Center = getWorldCenter();
		Center.Z = 0.0f;

		AHarvesterCreature* NewWorker = GetWorld()->SpawnActor<AHarvesterCreature>(WorkerClass, Center, FRotator::ZeroRotator);
		if (NewWorker)
		{
			NewWorker->Create(FVector2D(getTileX(), getTileY()), this);
			return NewWorker;
		}
	}

	return nullptr;
}

void AHarvesterTile::DeactivateWorker(AHarvesterCreature* Worker)
{
	if(Worker)
	{
		if(Worker->getHarvestTile())
		{
			Worker->getHarvestTile()->ClearHarvester();
		}
		
		Worker->DeactivateCreature();
		Worker->SetActorLocation(getWorldCenter() - FVector(0, 0, 1000));
	}
}

void AHarvesterTile::ActivateWorker(AHarvesterCreature* Worker)
{
	if(Worker)
	{
		Worker->ActivateCreature();
		Worker->SetActorLocation(getWorldCenter());
	}
}

bool AHarvesterTile::StoreItem()
{
	if (HasInventorySpace())
	{
		//Add a item to the inventory
		CurrentInventory++;

		if (getGamemode())
		{
			getGamemode()->AddFloatingItem(ProducedItems, 1, GetActorLocation());
		}

		return true;
	}

	return false;
}

void AHarvesterTile::TransferInventory(AHaulerCreature* Hauler)
{
	Super::TransferInventory(Hauler);

	//Send all workers back to work
	for(int32 i = 0; i < Workers.Num(); i++)
	{
		AHarvesterCreature* Worker = Workers[i];
		if(Worker)
		{
			if(Worker->getStatus() == ECreatureStatus::Deactivated)
			{
				UTileData* Tile = getNextHarvestTile();
				if(!Tile)
				{
					//No valid tile was found. Return
					return;
				}

				ActivateWorker(Worker);

				Tile->GiveHarvester();
				Worker->GiveHarvestJob(Tile);
			}
		}
	}
}

void AHarvesterTile::OnWorkerReturn(AHarvesterCreature* ReturningWorker)
{
	if(ReturningWorker)
	{
		if(ReturningWorker->hasHarvested())
		{
			ReturningWorker->ResetHasHarvested();
			
			if(StoreItem())
			{
				//Stored the item sucessfully
				if(HasInventorySpace())
				{
					//Get the remaining amount that we can still store
					int32 Remaining = InventorySize - CurrentInventory;

					int32 WorkersGathering = 0;
					for(int32 i = 0; i < Workers.Num(); i++)
					{
						AHarvesterCreature* Worker = Workers[i];
						if(Worker)
						{
							if (Worker != ReturningWorker)
							{
								if (Worker->getStatus() == ECreatureStatus::Harvesting || Worker->getStatus() == ECreatureStatus::ReturningGoods)
								{
									WorkersGathering++;
								}
							}
						}
					}

					DebugWarning("Remaining Space " + FString::FromInt(Remaining));
					DebugWarning("Workers working " + FString::FromInt(WorkersGathering));
					if(Remaining > WorkersGathering)
					{
						//This worker is good to go.

						//Check if the workertile is still good
						if (HarvesterType == EHarvesterType::Rescource) 
						{
							if (ReturningWorker->getHarvestTile())
							{
								if(!ReturningWorker->getHarvestTile()->HasRescource())
								{
									//Check if we can give this harvester a new tile
									UTileData* NewTile = getNextHarvestTile();
									if(NewTile)
									{
										NewTile->GiveHarvester();
										ReturningWorker->getHarvestTile()->ClearHarvester();
										ReturningWorker->GiveHarvestJob(NewTile);
										return;
									}else
									{
										//Deactivate the worker
										DeactivateWorker(ReturningWorker);
									}
								}
							}
						}
						ReturningWorker->GiveHarvestJob(ReturningWorker->getHarvestTile());
					}else
					{
						//We dont need to send him out again
						DeactivateWorker(ReturningWorker);
					}

				}else
				{
					//We have no more space left. Deactivate this worker
					DeactivateWorker(ReturningWorker);
				}
			}else
			{
				//We failed to store a item. Deactivate the worker
				DeactivateWorker(ReturningWorker);
			}
		}else
		{
			//This worker somehow returned without getting anything usefull. Deactivate him
			DeactivateWorker(ReturningWorker);
		}
	}
}

void AHarvesterTile::UpdateHarvestableTiles()
{
	
}

UTileData* AHarvesterTile::getNextHarvestTile()
{
	UTileData* BestTile = nullptr;
	for(int32 i = 0; i < HarvestAbleTiles.Num(); i++)
	{
		UTileData* Tile = HarvestAbleTiles[i];
		if(Tile)
		{
			//If there is already someone working on this tile
			if(Tile->HasHarvester())
			{
				continue;
			}

			//If we are a rescource harvester and the tile has not a rescource
			if(HarvesterType == EHarvesterType::Rescource)
			{
				if(!Tile->HasRescource())
				{
					continue;
				}
			}

			//Initial BestTile
			if(!BestTile)
			{
				BestTile = Tile;
				continue;
			}

			int32 OldDist = UTableHelper::getDistance(getTileX(), getTileY(), BestTile->getX(), BestTile->getY());
			int32 NewDist = UTableHelper::getDistance(getTileX(), getTileY(), Tile->getX(), Tile->getY());

			if(NewDist < OldDist)
			{
				BestTile = Tile;
			}
		}
	}

	return BestTile;
}

bool AHarvesterTile::HasInventorySpace()
{
	return (CurrentInventory < InventorySize);
}

int32 AHarvesterTile::getBuildGridRadius()
{
	if(BuildingData.ID != NAME_None)
	{
		return SearchRangeInTiles;
	}

	return Super::getBuildGridRadius();
}

EItem AHarvesterTile::getItemType()
{
	return ProducedItems;
}

TArray<FProductionItem> AHarvesterTile::getInputItems()
{
	return InputItemsData;
}

TArray<FProductionItem> AHarvesterTile::getOutputItems()
{
	return OutputItemsData;
}
