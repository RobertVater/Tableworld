// Copyright by Robert Vater (Gunschlinger)

#include "HarvesterBuilding.h"
#include "World/TableWorldTable.h"
#include "Creature/HarvesterCreature.h"
#include "../TileData.h"
#include "Core/TableGamemode.h"
#include "Core/TableGameInstance.h"
#include "Component/WorkerComponent.h"

AHarvesterTile::AHarvesterTile()
{
	WorkerComponent = CreateDefaultSubobject<UWorkerComponent>(TEXT("WorkerComponent"));
	
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
			HarvestAbleTiles = getTable()->getRescourcesInRadius(getCenterX(), getCenterY(), getBuildGridRadius(), HarvestRescources);
			break;

		case EHarvesterType::Tile:
			HarvestAbleTiles = getTable()->getTilesInRadius(getCenterX(), getCenterY(), getBuildGridRadius(), HarvestTile, true);
			break;
		}

		WorkerComponent->Init(this);

		if (!bLoadBuilding) 
		{
			WorkerComponent->SpawnAllWorkers();
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

	TArray<AWorkerCreature*> Workers = getWorkers();

	//Move all workers back home. No matter what they do
	for(int32 i = 0; i < Workers.Num(); i++)
	{
		AHarvesterCreature* Worker = Cast<AHarvesterCreature>(Workers[i]);
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
	TArray<AWorkerCreature*> Workers = getWorkers();
	for (int32 i = 0; i < Workers.Num(); i++)
	{
		AHarvesterCreature* Worker = Cast<AHarvesterCreature>(Workers[i]);
		if (Worker)
		{
			GiveWork(Worker, true);
		}
	}
}

bool AHarvesterTile::GiveWork(AHarvesterCreature* Worker, bool bForceWork)
{
	if(Worker)
	{
		UTileData* HarvestTile = Worker->getHarvestTile();
		if(HarvestTile)
		{
			if (!bForceWork) 
			{
				if (HarvestTile->getTileRescources() != getHarvestRescource() || HarvestTile->getTileRescourceAmount() <= 0)
				{
					HarvestTile->ClearHarvester();

					HarvestTile = getNextHarvestTile(getHarvestRescource());
				}
			}else
			{
				HarvestTile->ClearHarvester();
				HarvestTile = getNextHarvestTile(getHarvestRescource());
			}
		}else
		{
			HarvestTile = getNextHarvestTile(getHarvestRescource());
		}

		if (HarvestTile) 
		{
			HarvestTile->GiveHarvester();
			
			//Activate the worker
			Worker->ActivateCreature();

			//Tell the worker to harvest the tile
			Worker->GiveHarvestJob(HarvestTile, getProducedItem());
			return true;
		}else
		{
			Notification_NoRescources();
		}
	}

	return false;
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

void AHarvesterTile::AddRawItem()
{
	//Check if we could even store another item
	if (HasInventorySpace())
	{
		RawItems++;

		if(!GetWorldTimerManager().IsTimerActive(ItemProductionTimer))
		{
			GetWorldTimerManager().SetTimer(ItemProductionTimer, this, &AHarvesterTile::OnItemProduction, ProduceTime, false);
		}
	}
}

void AHarvesterTile::OnItemProduction()
{
	
}

bool AHarvesterTile::StoreItem(EItem Item)
{
	if (HasInventorySpace())
	{
		//Add a item to the inventory
		InventoryComponent->ModifyInventory(Item, 1);

		if (getGamemode())
		{
			getGamemode()->AddFloatingItem(Item, 1, GetActorLocation());

			if (getCurrentStorage() >= getMaxStorage())
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
	
	TArray<AWorkerCreature*> Workers = getWorkers();

	//Send all workers back to work
	for(int32 i = 0; i < Workers.Num(); i++)
	{
		AHarvesterCreature* Worker = Cast<AHarvesterCreature>(Workers[i]);
		if(Worker)
		{
			if(Worker->getStatus() == ECreatureStatus::Deactivated)
			{
				GiveWork(Worker);
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
			if (StoreItem(ReturningWorker->getHarvestItem()))
			{
				TrySendWorkerToWork(ReturningWorker);
				return;
			}
		}

		ReturningWorker->DeactivateCreature();
	}
}

void AHarvesterTile::OnInputItemChanged(int32 Index)
{
	HarvestRescourceIndex = Index;

	TArray<AWorkerCreature*> Workers = getWorkers();

	//Tell the workers currently going to work to get the new tiles
	for(int32 i = 0; i < Workers.Num(); i++)
	{
		AHarvesterCreature* Worker = Cast<AHarvesterCreature>(Workers[i]);
		if(Worker)
		{
			if(Worker->getStatus() == ECreatureStatus::Deactivated || Worker->getStatus() == ECreatureStatus::GoingToWork)
			{
				GiveWork(Worker, true);
			}
		}
	}
}

AHarvesterCreature* AHarvesterTile::getDeactivatedWorker()
{
	return Cast<AHarvesterCreature>(WorkerComponent->getDeactivatedWorker());
}

void AHarvesterTile::TrySendWorkerToWork(AHarvesterCreature* AvailableWorker)
{
	if (!AvailableWorker)return;

	//Try to determin if this worker should return to work.
	int32 RemainingSpace = getMaxStorage() - getCurrentStorage();

	if (RemainingSpace > 0)
	{
		TArray<AWorkerCreature*> Workers = getWorkers();

		//Check how many workers are currently getting wood
		int32 WorkingWorkers = 0;
		for (int32 i = 0; i < Workers.Num(); i++)
		{
			AHarvesterCreature* Worker = Cast<AHarvesterCreature>(Workers[i]);
			if (Worker)
			{
				if (Worker->getStatus() != ECreatureStatus::Deactivated && Worker->getStatus() != ECreatureStatus::Idle)
				{
					WorkingWorkers++;
				}
			}
		}

		if (AvailableWorker)
		{
			if (WorkingWorkers <= RemainingSpace)
			{
				//Its worth to send this worker back to work. 
				GiveWork(AvailableWorker);
				return;
			}
		}
	}

	AvailableWorker->DeactivateCreature();
}

void AHarvesterTile::UpdateHarvestableTiles()
{
	for(int32 i = 0; i < HarvestAbleTiles.Num(); i++)
	{
		UTileData* Tile = HarvestAbleTiles[i];
		if(Tile)
		{
			if(Tile->getTileRescources() == getHarvestRescource())
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

UTileData* AHarvesterTile::getNextHarvestTile(ETileRescources Rescource)
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

				if(Rescource != ETileRescources::None && Rescource != ETileRescources::Max)
				{
					if(Tile->getTileRescources() != Rescource)
					{
						continue;
					}
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
	return getProducedItem();
}

EItem AHarvesterTile::getProducedItem()
{
	if (HarvesterType == EHarvesterType::Tile)return EItem::None;
	
	if(ProducedItems.IsValidIndex(getHarvestRescourceIndex()))
	{
		return ProducedItems[getHarvestRescourceIndex()];
	}

	DebugError(GetName() + " failed to get produce a item because the item list is empty!");
	return EItem::None;
}

ETileRescources AHarvesterTile::getHarvestRescource()
{
	if (HarvesterType == EHarvesterType::Rescource)return ETileRescources::None;
	
	if (HarvestRescources.IsValidIndex(getHarvestRescourceIndex())) 
	{
		return HarvestRescources[getHarvestRescourceIndex()];
	}

	DebugError(GetName() + " failed to get a harvest rescource becasuse the list is empty!");
	return ETileRescources::None;
}

TArray<AWorkerCreature*> AHarvesterTile::getWorkers()
{
	return WorkerComponent->getWorkers();
}

TArray<FProductionItem> AHarvesterTile::getInputItems()
{
	return InputItemsData;
}

TArray<FProductionItem> AHarvesterTile::getOutputItems()
{
	return OutputItemsData;
}

TArray<ETileRescources> AHarvesterTile::getInputChoices()
{
	return HarvestRescources;
}

int32 AHarvesterTile::getHarvestRescourceIndex()
{
	return HarvestRescourceIndex;
}

void AHarvesterTile::SaveData_Implementation(UTableSavegame* Savegame)
{
	
}

FTableInfoPanel AHarvesterTile::getInfoPanelData_Implementation()
{
	FTableInfoPanel Data = Super::getInfoPanelData_Implementation();
	Data.WorkerComponent = WorkerComponent;

	//Input
	for (int32 i = 0; i < InputItemsData.Num(); i++) 
	{
		FProductionItem Item = InputItemsData[i];
		
		FTableInfo_Item InputItem;
		InputItem.Icon = Item.Icon;
		InputItem.Name = Item.Name;
		InputItem.Amount = Item.Amount;
		InputItem.RescourceChoices = getInputChoices();

		Data.InputItems.Add(InputItem);
	}

	//Output
	for (int32 i = 0; i < OutputItemsData.Num(); i++)
	{
		FProductionItem Item = OutputItemsData[i];

		FTableInfo_Item OutputItem;
		OutputItem.Icon = Item.Icon;
		OutputItem.Name = Item.Name;
		OutputItem.Amount = Item.Amount;

		Data.OutputItems.Add(OutputItem);
	}

	Data.PanelSize = FVector2D(350, 400);
	return Data;
}

FTableInfoPanel AHarvesterTile::getUpdateInfoPanelData_Implementation()
{
	FTableInfoPanel Data = Super::getUpdateInfoPanelData_Implementation();

	Data.InventoryComponent = InventoryComponent;

	return Data;
}
