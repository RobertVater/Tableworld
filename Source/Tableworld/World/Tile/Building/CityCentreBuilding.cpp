// Copyright by Robert Vater (Gunschlinger)

#include "CityCentreBuilding.h"
#include "World/TableWorldTable.h"
#include "HarvesterBuilding.h"
#include "Core/TableGamemode.h"
#include "../TileData.h"
#include "DrawDebugHelpers.h"
#include "BuildableTile.h"
#include "Creature/HaulerCreature.h"
#include "ProductionBuilding.h"
#include "../DirtRoadTile.h"

ACityCentreTile::ACityCentreTile()
{
	WorkerComponent = CreateDefaultSubobject<UWorkerComponent>(TEXT("WorkerComponent"));
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	InfluenceComponent = CreateDefaultSubobject<UInfluenceComponent>(TEXT("InfluenceComponent"));
	
	Tags.Add("StorageBuilding");
}

void ACityCentreTile::Place(FVector PlaceLoc, TArray<FVector2D> nPlacedOnTiles, FTableBuilding nBuildingData, bool bNewRotated, bool bLoadBuilding)
{
	Super::Place(PlaceLoc, nPlacedOnTiles, nBuildingData, bNewRotated, bLoadBuilding);

	WorkerComponent->Init(this);

	if (!bLoadBuilding)
	{
		WorkerComponent->SpawnAllWorkers();
		StartWork();
	}

	TArray<AWorkerCreature*> Workers = getWorkers();
	for (int32 i = 0; i < Workers.Num(); i++)
	{
		AHaulerCreature* Hauler = Cast<AHaulerCreature>(Workers[i]);
		if (Hauler)
		{
			Hauler->Event_HaulerReturnedHome.AddDynamic(this, &ACityCentreTile::OnHaulerReturnHome);
			Hauler->Event_HaulerReachedTarget.AddDynamic(this, &ACityCentreTile::OnHaulReachedTarget);
		}
	}
}

void ACityCentreTile::StartWork()
{
	Super::StartWork();

	GetWorldTimerManager().SetTimer(RescourceCheckTimer, this, &ACityCentreTile::OnRescourceCheck, RescourceCheckTime, true);
}

void ACityCentreTile::StopWork()
{
	Super::StopWork();

	GetWorldTimerManager().ClearTimer(RescourceCheckTimer);
}

int32 ACityCentreTile::getBuildGridRadius()
{
	if (BuildingData.ID != NAME_None)
	{
		return InfluenceComponent->InfluenceRange;
	}

	return Super::getBuildGridRadius();
}

void ACityCentreTile::OnRescourceCheck()
{
	FVector2D InTile;
	FVector2D OutTile;

	//Check if we have a valid hauler with nothing todo
	AHaulerCreature* FoundHauler = nullptr;

	//Get a deactivated hauler and send him to the target!
	TArray<AWorkerCreature*> Workers = getWorkers();

	for (int32 i = 0; i < Workers.Num(); i++)
	{
		AHaulerCreature* HaulerWorker = Cast<AHaulerCreature>(Workers[i]);
		if (HaulerWorker)
		{
			if (HaulerWorker->getStatus() == ECreatureStatus::Deactivated)
			{
				FoundHauler = HaulerWorker;
				break;
			}
		}
	}

	if (FoundHauler) 
	{
		//We found a valid hauler for the job

		AProductionBuilding* Building = getValidHaulGoal(InTile, OutTile);
		if (Building)
		{
			Building->SetHaulLocked(true);

			if (getGamemode())
			{
				if (getGamemode()->getTable())
				{
					UTileData* StartTile = getGamemode()->getTile((int32)OutTile.X, (int32)OutTile.Y);
					UTileData* EndTile = getGamemode()->getTile((int32)InTile.X, (int32)InTile.Y);
					if (StartTile && EndTile)
					{
						//Send the hauler to work.
						FoundHauler->SetActorLocation(StartTile->getWorldCenter());

						FoundHauler->ActivateCreature();
						FoundHauler->GiveHaulJob(Building->getUID(), getUID(), EndTile, StartTile);
					}
				}
			}
		}
	}
}

void ACityCentreTile::OnHaulerReturnHome(AHaulerCreature* nHauler)
{
	if(nHauler)
	{
		if(getGamemode())
		{
			for(auto Elem : nHauler->InventoryComponent->getInventory())
			{
				EItem Item = Elem.Key;
				int32 Amount = Elem.Value;
				
				getGamemode()->AddFloatingItem(Item, Amount, getWorldCenter());

				//Add to local inventory
				ModifyInventory(Item, Amount);

				//Add the item to the global inventory
				getGamemode()->ModifyRescource(Item, Amount);

				DebugError("City " + FString::FromInt((int32)Elem.Key) + " x" + FString::FromInt(Elem.Value));
			}
		}

		nHauler->ClearInventory();
		nHauler->DeactivateCreature();
	}
}

void ACityCentreTile::OnHaulReachedTarget(AHaulerCreature* nHauler)
{
	if (!nHauler)return;
	if (!getTable())return;

	AProductionBuilding* InventoryBuilding = Cast<AProductionBuilding>(getTable()->getBuildingWithID(nHauler->getTargetBuildingUID()));
	if(InventoryBuilding)
	{
		InventoryBuilding->TransferInventory(nHauler);
		nHauler->GiveReturnJob();

		return;
	}

	nHauler->GiveReturnJob();
}

void ACityCentreTile::ModifyInventory(EItem Item, int32 Amount)
{
	InventoryComponent->ModifyInventory(Item, Amount);
}

bool ACityCentreTile::ReserveItems(TMap<EItem, int32> Items, FName UID)
{
	return InventoryComponent->AddReservedItem(Items, UID);
}

void ACityCentreTile::ClearReserveItems(FName UID)
{
	InventoryComponent->ClearReservedItem(UID);
}

TArray<AWorkerCreature*> ACityCentreTile::getWorkers()
{
	return WorkerComponent->getWorkers();
}

AProductionBuilding* ACityCentreTile::getValidHaulGoal(FVector2D& InTile, FVector2D& OutTile)
{
	if (getGamemode())
	{
		if (getGamemode()->getTable())
		{
			TArray<UTileData*> TilesAroundUs = getTilesAroundUs(true);

			//Check if we are connected to a road
			if(!isConnectedToRoad())
			{
				return nullptr;
			}

			AProductionBuilding* FoundHaulTarget = nullptr;
			UTileData* FoundInTile = nullptr;
			UTileData* FoundEndTile = nullptr;

			TArray<ABuildableTile*> Buildings = getGamemode()->getTable()->getBuildings();
			for (int32 i = 0; i < Buildings.Num(); i++)
			{
				ABuildableTile* Building = Buildings[i];
				if (Building)
				{
					Building->getTilesAroundUs(true);
					
					//Check if the building is a production building
					if (!Building->IsA(AProductionBuilding::StaticClass()))
					{
						continue;
					}

					//Check if the target building is connected to a road
					if(!Building->isConnectedToRoad())
					{
						continue;
					}

					if (Building->isHaulerComming())
					{
						//Ignore buildings that already have a hauler comming
						continue;
					}

					if (!getTable()->InInfluenceRange(getCenterX(), getCenterY(), getBuildGridRadius(), Building->getTileX(), Building->getTileY(), Building->getBuildingSize()))
					{
						//Do not care about buildings that are outside your range
						continue;
					}

					AProductionBuilding* InventoryTile = Cast<AProductionBuilding>(Building);
					if (InventoryTile)
					{
						//If this building has some items stored
						if (!InventoryTile->canBeHauled())
						{
							//Check the next building
							continue;
						}

						//Check if we can reach any of the tiles around the building
						TArray<UTileData*> Tiles = Building->getTilesAroundUs(true);

						for (int32 j = 0; j < Tiles.Num(); j++)
						{
							UTileData* Tile = Tiles[j];
							if (!Tile)
							{
								continue;
							}

							if(Tile->getTileType() != ETileType::DirtRoad)
							{
								continue;
							}

							if (Tile->IsBlocked())
							{
								continue;
							}

							for (int32 k = 0; k < TilesAroundUs.Num(); k++)
							{
								UTileData* OurTile = TilesAroundUs[k];
								if (OurTile)
								{
									if (OurTile->getTileType() != ETileType::DirtRoad)
									{
										continue;
									}

									if (OurTile->IsBlocked())
									{
										continue;
									}

									TArray<UTileData*> Path = getGamemode()->getTable()->FindPathRoad(Tile, OurTile, false);
									if (Path.Num() > 0)
									{
										if(!FoundHaulTarget)
										{
											FoundHaulTarget = InventoryTile;
											FoundInTile = Tile;
											FoundEndTile = OurTile;

											continue;
										}

										//Check if this building has more items stored currently
										int32 OldStorage = FoundHaulTarget->getStoredItemCount();
										int32 NewStorage = InventoryTile->getStoredItemCount();

										if(NewStorage > OldStorage)
										{
											FoundHaulTarget = InventoryTile;
											FoundInTile = Tile;
											FoundEndTile = OurTile;
										}
									}
								}
							}
						}
					}
				}
			}

			if(FoundHaulTarget)
			{
				InTile = FoundInTile->getPositionAsVector();
				OutTile = FoundEndTile->getPositionAsVector();

				//Found a valid building!
				return FoundHaulTarget;
			}
		}

	}

	return nullptr;
}

TMap<EItem, int32> ACityCentreTile::getStoredItems()
{
	return InventoryComponent->getInventory();
}

bool ACityCentreTile::HasItems(TMap<EItem, int32> Items, bool bIgnoreReserved)
{
	return InventoryComponent->hasItems(Items, bIgnoreReserved);
}

void ACityCentreTile::SaveData_Implementation(UTableSavegame* Savegame)
{
	
}

FTableInfoPanel ACityCentreTile::getInfoPanelData_Implementation()
{
	FTableInfoPanel Data = Super::getInfoPanelData_Implementation();

	Data.InventoryComponent = InventoryComponent;
	Data.WorkerComponent = WorkerComponent;

	return Data;
}

FTableInfoPanel ACityCentreTile::getUpdateInfoPanelData_Implementation()
{
	FTableInfoPanel Data = Super::getUpdateInfoPanelData_Implementation();

	Data.InventoryComponent = InventoryComponent;

	return Data;
}
