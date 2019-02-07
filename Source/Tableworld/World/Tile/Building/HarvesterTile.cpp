// Copyright by Robert Vater (Gunschlinger)

#include "HarvesterTile.h"
#include "World/TableWorldTable.h"
#include "Creature/HarvesterCreature.h"
#include "../TileData.h"
#include "Core/TableGamemode.h"
#include "Core/TableGameInstance.h"

AHarvesterTile::AHarvesterTile()
{
	Tags.Add("HarvesterBuilding");
}

void AHarvesterTile::Place(FVector PlaceLoc, TArray<FVector2D> nPlacedOnTiles, FTableBuilding nBuildingData, bool bNewRotated, bool bLoadBuilding)
{
	Super::Place(PlaceLoc, nPlacedOnTiles, nBuildingData, bNewRotated, bLoadBuilding);

	if (getTable())
	{
		//Get the tiles we are allowed  to harvest
		switch (HarvesterType)
		{
		case EHarvesterType::Rescource:
			HarvestAbleTiles = getTable()->getRescourcesInRadius(getCenterX(), getCenterY(), getBuildGridRadius(), HarvestRescource);
			break;

		case EHarvesterType::Tile:
			HarvestAbleTiles = getTable()->getTilesInRadius(getCenterX(), getCenterY(), getBuildGridRadius(), HarvestTile, true);
			break;
		}

		if (!bLoadBuilding) 
		{
			SpawnWorkers();
			StartWork();
		}
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
	//Send all workers to work
	for(int32 i = 0; i < Workers.Num(); i++)
	{
		AHarvesterCreature* Worker = Workers[i];
		if(Worker)
		{
			//Give workers who dont have a valid tile yet something todo
			if (!Worker->getHarvestTile()) 
			{
				//Get a suitable tile for the worker
				UTileData* NewHarvestTile = getNextHarvestTile();
				if (NewHarvestTile == nullptr)
				{
					//Deactivate this worker.
					Worker->DeactivateCreature();
					continue;
				}

				//Activate workers
				Worker->ActivateCreature();

				NewHarvestTile->GiveHarvester();
				Worker->GiveHarvestJob(NewHarvestTile);
			}
		}
	}
}

void AHarvesterTile::SpawnWorkers()
{
	for(int32 i = 0; i < MaxWorkerCount; i++)
	{
		AHarvesterCreature* Worker = SpawnWorker();
	}
}

AHarvesterCreature* AHarvesterTile::SpawnWorker()
{
	if (WorkerClass)
	{
		AHarvesterCreature* NewWorker = GetWorld()->SpawnActor<AHarvesterCreature>(WorkerClass, getWorldCenter(), FRotator::ZeroRotator);
		if (NewWorker)
		{
			NewWorker->DeactivateCreature();
			NewWorker->Create(FVector2D(getTileX(), getTileY()), this);

			Workers.Add(NewWorker);
			return NewWorker;
		}
	}

	return nullptr;
}

void AHarvesterTile::Notification_FullStorage()
{
	if (getGamemode())
	{
		FTableNotification Data;
		Data.bForcePause = false;
		Data.NotificationLocation = getWorldCenter();
		Data.NotificationType = ETableNotificationType::Problem;

		Data.NotificationTitle = FText::AsCultureInvariant("Full Storage");
		Data.NotificationText = FText::AsCultureInvariant("A Harvester has stopped working because his storage is full!");

		getGamemode()->AddNotification(Data);
	}
}

void AHarvesterTile::Notification_NoRescources()
{
	if (getGamemode())
	{
		FTableNotification Data;
		Data.bForcePause = false;
		Data.NotificationLocation = getWorldCenter();
		Data.NotificationType = ETableNotificationType::Problem;

		Data.NotificationTitle = FText::AsCultureInvariant("No Rescources!");
		Data.NotificationText = FText::AsCultureInvariant("A Harvester has stopped working because there are no Rescources around the building!");

		getGamemode()->AddNotification(Data);
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

			if (CurrentInventory >= InventorySize)
			{
				//Full Storage
				Notification_FullStorage();
			}
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

				Worker->ActivateCreature();

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
			
			//Try to store a new item.
			if(StoreItem())
			{
				//Try to determin if this worker should return to work.
				int32 RemainingSpace = getMaxStorage() - getCurrentStorage();

				DebugError("Remaming Space " + FString::FromInt(RemainingSpace));

				if(RemainingSpace > 0)
				{
					//Check how many workers are currently getting wood
					int32 WorkingWorkers = 0;
					for(int32 i = 0; i < Workers.Num(); i++)
					{
						AHarvesterCreature* Worker = Workers[i];
						if(Worker)
						{
							if(Worker->getStatus() != ECreatureStatus::Deactivated && Worker->getStatus() != ECreatureStatus::Idle)
							{
								WorkingWorkers++;
							}
						}
					}
					DebugError("Workers " + FString::FromInt(WorkingWorkers));


					if(WorkingWorkers <= RemainingSpace)
					{
						UTileData* HarvestTile = ReturningWorker->getHarvestTile();
						if(HarvestTile)
						{
							if(HarvestTile->getTileRescourceAmount() <= 0)
							{
								HarvestTile->ClearHarvester();
								UpdateHarvestableTiles();
								
								HarvestTile = getNextHarvestTile();
							}
						}else
						{
							HarvestTile = getNextHarvestTile();
						}

						if(HarvestTile)
						{
							HarvestTile->GiveHarvester();
							ReturningWorker->GiveHarvestJob(HarvestTile);
							return;
						}else
						{
							Notification_NoRescources();
						}
					}
				}
			}
		}

		ReturningWorker->DeactivateCreature();
	}
}

void AHarvesterTile::UpdateHarvestableTiles()
{
	for(int32 i = 0; i < HarvestAbleTiles.Num(); i++)
	{
		UTileData* Tile = HarvestAbleTiles[i];
		if(Tile)
		{
			if(Tile->getTileRescources() == HarvestRescource)
			{
				continue;
			}

			if(Tile->getTileRescourceAmount() > 0)
			{
				continue;
			}

			HarvestAbleTiles.Remove(Tile);
		}
	}
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
	return SearchRangeInTiles - 1;
}

FColor AHarvesterTile::getGridColor()
{
	return FColor::Green;
}

float AHarvesterTile::getGridHeigth()
{
	return 5.0f;
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

void AHarvesterTile::LoadData(FTableSaveHarvesterBuilding Data)
{
	UID = Data.UID;
	CurrentInventory = Data.CurrentInventory;

	//Override the current workers with our new data
	for(int32 i = 0; i < Data.Workers.Num(); i++)
	{
		FTableSaveHarvesterCreature WorkerData = Data.Workers[i];

		AHarvesterCreature* NewWorker = SpawnWorker();
		if(NewWorker)
		{
			DebugWarning("Loaded new Worker!");
			NewWorker->LoadData(WorkerData);

			Workers.Add(NewWorker);
		}
	}

	StartWork();
}

void AHarvesterTile::SaveData_Implementation(UTableSavegame* Savegame)
{
	FTableSaveHarvesterBuilding Harvester;
	
	Harvester.UID = UID;
	Harvester.TileX = getTileX();
	Harvester.TileY = getTileY();
	Harvester.Rotation = GetActorRotation().Yaw;
	Harvester.bRotated = bRotated;
	Harvester.BuildingID = getBuildingData().ID;
	Harvester.CurrentInventory = getCurrentStorage();

	//Save our workers
	for(int32 i = 0; i < Workers.Num(); i++)
	{
		AHarvesterCreature* Worker = Workers[i];
		if(Worker)
		{
			Harvester.Workers.Add(Worker->getSaveData());
		}
	}

	Savegame->SavedHarvesters.Add(Harvester);
}
