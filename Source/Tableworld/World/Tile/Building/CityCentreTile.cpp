// Copyright by Robert Vater (Gunschlinger)

#include "CityCentreTile.h"
#include "World/TableWorldTable.h"
#include "HarvesterTile.h"
#include "Core/TableGamemode.h"
#include "../TileData.h"
#include "DrawDebugHelpers.h"
#include "BuildableTile.h"
#include "Creature/HaulerCreature.h"
#include "InventoryTile.h"
#include "../DirtRoadTile.h"

ACityCentreTile::ACityCentreTile()
{
	Tags.Add("StorageBuilding");
}

void ACityCentreTile::Place(FVector PlaceLoc, TArray<FVector2D> nPlacedOnTiles, FTableBuilding nBuildingData, bool bNewRotated, bool bLoadBuilding)
{
	Super::Place(PlaceLoc, nPlacedOnTiles, nBuildingData, bNewRotated, bLoadBuilding);

	if (!bLoadBuilding)
	{
		SpawnHaulers();
		StartWork();
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
		return InfluenceRadius;
	}

	return Super::getBuildGridRadius();
}

void ACityCentreTile::OnRescourceCheck()
{
	DebugError("Check");

	FVector2D InTile;
	FVector2D OutTile;

	//Check if we have a valid hauler with nothing todo
	AHaulerCreature* FoundHauler = nullptr;

	//Get a deactivated hauler and send him to the target!
	for (int32 i = 0; i < Workers.Num(); i++)
	{
		AHaulerCreature* HaulerWorker = Workers[i];
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
		DebugError("Found Hauler");

		AInventoryTile* Building = getValidHaulGoal(InTile, OutTile);
		if (Building)
		{
			DebugError("Found Building");

			Building->SetHaulLocked(true);

			if (getGamemode())
			{
				if (getGamemode()->getTable())
				{
					DebugError("Found Table");

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
			EItem HaulItem = EItem::None;
			int32 HaulAmount = 0;

			if (nHauler->getItemZero(HaulItem, HaulerAmount))
			{
				getGamemode()->AddFloatingItem(HaulItem, HaulerAmount, getWorldCenter());

				//Add to local inventory
				ModifyInventory(HaulItem, HaulerAmount);

				//Add the item to the global inventory
				getGamemode()->ModifyRescource(HaulItem, HaulerAmount);
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

	AInventoryTile* InventoryBuilding = Cast<AInventoryTile>(getTable()->getBuildingWithID(nHauler->getTargetBuildingUID()));
	if(InventoryBuilding)
	{
		InventoryBuilding->TransferInventory(nHauler);
		nHauler->GiveReturnJob();
	}
}

void ACityCentreTile::ModifyInventory(EItem Item, int32 Amount)
{
	if (Item == EItem::None && Item == EItem::Max)return;

	if(!StoredItems.Contains(Item))
	{
		if (Amount > 0) 
		{
			StoredItems.Add(Item, Amount);
		}

		return;
	}

	int32 OldAmount = StoredItems.FindRef(Item) + Amount;

	if(OldAmount <= 0)
	{
		StoredItems.Remove(Item);
		return;
	}

	StoredItems.Emplace(Item, OldAmount);
}

bool ACityCentreTile::ReserveItems(TMap<EItem, int32> Items, FName UID)
{
	if(HasItems(Items))
	{
		//Reserve the items
		for(auto Elem : Items)
		{
			EItem Item = Elem.Key;
			int32 Amount = Elem.Value;

			FReservedItem ReservedItem;
			ReservedItem.Item = Item;
			ReservedItem.Amount = Amount;
			ReservedItem.BuildingUID = UID;

			ReservedItems.Add(ReservedItem);
		}

		return true;
	}

	return false;
}

void ACityCentreTile::ClearReserveItems(FName UID)
{
	if (UID != NAME_None)
	{
		for (int32 i = 0; i < ReservedItems.Num(); i++)
		{
			FReservedItem Item = ReservedItems[i];
			if(Item.BuildingUID.IsEqual(UID))
			{
				ReservedItems.RemoveAt(i);
			}
		}
	}
}

void ACityCentreTile::SpawnHaulers()
{
	for(int32 i = 0; i < HaulerAmount; i++)
	{
		AHaulerCreature* Hauler = SpawnWorker(getWorldCenter());
	}
}

AHaulerCreature* ACityCentreTile::SpawnWorker(FVector SpawnLoc)
{
	AHaulerCreature* NewWorker = GetWorld()->SpawnActor<AHaulerCreature>(HaulerClass, SpawnLoc, FRotator::ZeroRotator);
	if(NewWorker)
	{
		//Bind events
		NewWorker->Event_HaulerReturnedHome.AddDynamic(this, &ACityCentreTile::OnHaulerReturnHome);
		NewWorker->Event_HaulerReachedTarget.AddDynamic(this, &ACityCentreTile::OnHaulReachedTarget);

		NewWorker->DeactivateCreature();
		Workers.Add(NewWorker);
	}

	return NewWorker;
}

AInventoryTile* ACityCentreTile::getValidHaulGoal(FVector2D& InTile, FVector2D& OutTile)
{
	DebugError("----0");

	if (getGamemode())
	{
		DebugError("----1");
		if (getGamemode()->getTable())
		{
			DebugError("----2");
			TArray<UTileData*> TilesAroundUs = getTilesAroundUs(true);

			//Check if we are connected to a road
			if(!isConnectedToRoad())
			{
				return nullptr;
			}
			DebugError("----3");

			TArray<ABuildableTile*> Buildings = getGamemode()->getTable()->getBuildings();
			for (int32 i = 0; i < Buildings.Num(); i++)
			{
				ABuildableTile* Building = Buildings[i];
				if (Building)
				{
					DebugError("----4");
					
					Building->getTilesAroundUs(true);
					
					//Check if the building is a inventory building
					if (!Building->IsA(AInventoryTile::StaticClass()))
					{
						continue;
					}
					DebugError("----5");

					if (Building->isHaulerComming())
					{
						//Ignore buildings that already have a hauler comming
						continue;
					}
					DebugError("----6");

					if (!getTable()->InInfluenceRange(getCenterX(), getCenterY(), getBuildGridRadius(), Building->getTileX(), Building->getTileY(), Building->getBuildingSize()))
					{
						//Do not care about buildings that are outside your range
						continue;
					}
					DebugError("----7");

					//Check if the target building is connected to a road
					if(!Building->isConnectedToRoad())
					{
						continue;
					}
					DebugError("----8");

					AInventoryTile* InventoryTile = Cast<AInventoryTile>(Building);
					if (InventoryTile)
					{
						DebugError("----9");
						//If this building has some items stored
						if (!InventoryTile->canBeHauled())
						{
							//Check the next building
							continue;
						}
						DebugError("----10");

						//Check if we can reach any of the tiles around the building
						TArray<UTileData*> Tiles = Building->getTilesAroundUs(true);

						for (int32 j = 0; j < Tiles.Num(); j++)
						{
							UTileData* Tile = Tiles[j];
							if (!Tile)
							{
								continue;
							}
							DebugError("----11");

							if(Tile->getTileType() != ETileType::DirtRoad)
							{
								continue;
							}
							DebugError("----12");

							if (Tile->IsBlocked())
							{
								continue;
							}
							DebugError("----13");

							for (int32 k = 0; k < TilesAroundUs.Num(); k++)
							{
								UTileData* OurTile = TilesAroundUs[k];
								if (OurTile)
								{
									DebugError("----14");
									if (OurTile->getTileType() != ETileType::DirtRoad)
									{
										continue;
									}
									DebugError("----15");

									if (OurTile->IsBlocked())
									{
										continue;
									}
									DebugError("----16");

									TArray<UTileData*> Path = getGamemode()->getTable()->FindPathRoad(Tile, OurTile, false);
									if (Path.Num() > 0)
									{
										DebugError("----17 END");
										InTile = Tile->getPositionAsVector();
										OutTile = OurTile->getPositionAsVector();

										//Found a valid building!
										return InventoryTile;
									}
								}
							}
						}
					}
				}
			}
		}

	}

	return nullptr;
}

TMap<EItem, int32> ACityCentreTile::getStoredItems()
{
	return StoredItems;
}

bool ACityCentreTile::HasItems(TMap<EItem, int32> Items, bool bIgnoreReserved)
{
	TMap<EItem, int32> Inventory = StoredItems;
	if(!bIgnoreReserved)
	{
		//Subtract the reserved items from the inventory
		for(int32 i = 0; i < ReservedItems.Num(); i++)
		{
			FReservedItem Reserve = ReservedItems[i];
			if(Inventory.Contains(Reserve.Item))
			{
				int32 Amount = Inventory.FindRef(Reserve.Item);
				if(Reserve.Amount >= Amount)
				{
					Inventory.Remove(Reserve.Item);
				}else
				{
					int32 NewAmount = Amount - Reserve.Amount;
					Inventory.Emplace(Reserve.Item, NewAmount);
				}
			}
		}
	}
	
	for(auto Elem : Items)
	{
		EItem Item = Elem.Key;
		int32 Amount = Elem.Value;

		if(Inventory.Contains(Item))
		{
			int32 FoundAmount = StoredItems.FindRef(Item);
			if(FoundAmount < Amount)
			{
				return false;
			}
		}else
		{
			return false;
		}
	}

	return true;
}

void ACityCentreTile::LoadData(FTableSaveCityCenterBuilding Data)
{
	UID = Data.UID;
	StoredItems = Data.StoredItems;
	ReservedItems = Data.ReservedItems;

	//Spawn all haulers.
	SpawnHaulers();
	//Override the current workers with our new data
	for (int32 i = 0; i < Data.Workers.Num(); i++)
	{
		FTableSaveHaulerCreature WorkerData = Data.Workers[i];

		if (Workers.IsValidIndex(i)) 
		{
			AHaulerCreature* NewWorker = Workers[i];
			if (NewWorker)
			{
				NewWorker->LoadData(WorkerData);
				NewWorker->SetActorLocation(WorkerData.Location);

				Workers.Add(NewWorker);
			}
		}
	}

	StartWork();
}

void ACityCentreTile::SaveData_Implementation(UTableSavegame* Savegame)
{
	if(Savegame)
	{
		FTableSaveCityCenterBuilding CityCenter;
		CityCenter.UID = UID;
		CityCenter.BuildingID = getBuildingData().ID;
		CityCenter.TileX = getTileX();
		CityCenter.TileY = getTileY();
		CityCenter.Rotation = GetActorRotation().Yaw;
		CityCenter.bRotated = bRotated;
		CityCenter.StoredItems = StoredItems;
		CityCenter.ReservedItems = ReservedItems;

		//Save Workers
		for(int32 i = 0; i < Workers.Num(); i++)
		{
			AHaulerCreature* Worker = Workers[i];
			if(Worker)
			{
				FTableSaveHaulerCreature WorkerData = Worker->getSaveData();
				CityCenter.Workers.Add(WorkerData);
			}
		}

		Savegame->SavedCityCenters.Add(CityCenter);
	}
}
