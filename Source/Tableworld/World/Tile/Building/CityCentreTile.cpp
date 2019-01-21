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
		return InfluenceRadius + (int32)(BuildingData.BuildingSize.X + BuildingData.BuildingSize.Y);
	}

	return Super::getBuildGridRadius();
}

bool ACityCentreTile::InInfluenceRange(int32 X, int32 Y, FVector2D Size)
{
	if(getTable())
	{
		bool bInRange = true;
		for(int32 x = 0; x < Size.X; x++)
		{
			for (int32 y = 0; y < Size.Y; y++)
			{
				int32 CheckX = X + x;
				int32 CheckY = Y + y;

				int32 Distance = getTable()->getDistance(getTileX(), getTileY(), CheckX, CheckY);
				if(Distance > InfluenceRadius)
				{
					bInRange = false;
				}
			}
		}

		return bInRange;
	}

	return false;
}

void ACityCentreTile::OnRescourceCheck()
{
	//Try to spawn a hauler
	if (Workers.Num() < HaulerAmount) 
	{
		FVector2D InTile;
		FVector2D OutTile;

		AInventoryTile* Building = getValidHaulGoal(InTile,OutTile);
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
						//Spawn a hauler and send him to haul the item
						AHaulerCreature* Hauler = GetWorld()->SpawnActor<AHaulerCreature>(HaulerClass, StartTile->getWorldCenter(), FRotator::ZeroRotator);
						if(Hauler)
						{
							Hauler->GiveHaulJob(Building, this, EndTile, StartTile);
							Workers.Add(Hauler);
						}
					}
				}
			}
		}
	}
}

void ACityCentreTile::OnHaulCompleted(AHaulerCreature* nHauler)
{
	if(nHauler)
	{
		//TODO Get the items!
		if(getGamemode())
		{
			EItem HaulItem = nHauler->getHaulItem();
			int32 HaulAmount = nHauler->getHaulAmount();

			DebugWarning("Haul completed " + FString::FromInt(HaulAmount));

			getGamemode()->AddFloatingItem(HaulItem, nHauler->getHaulAmount(), getWorldCenter());

			//Add to local inventory
			ModifyInventory(HaulItem, nHauler->getHaulAmount());

			//Add the item to the global inventory
			getGamemode()->ModifyRescource(HaulItem, nHauler->getHaulAmount());
		}

		Workers.Remove(nHauler);
		nHauler->Destroy();
	}
}

void ACityCentreTile::ModifyInventory(EItem Item, int32 Amount)
{
	if (Item == EItem::None && Item == EItem::Max)return;
	
	if(!StoredItems.Contains(Item))
	{
		StoredItems.Add(Item, Amount);
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

AInventoryTile* ACityCentreTile::getValidHaulGoal(FVector2D& InTile, FVector2D& OutTile)
{
	if (getGamemode())
	{
		if (getGamemode()->getTable())
		{
			TArray<UTileData*> TilesAroundUs = getTilesAroundUs(true);

			TArray<ABuildableTile*> Buildings = getGamemode()->getTable()->getBuildings();
			for (int32 i = 0; i < Buildings.Num(); i++)
			{
				ABuildableTile* Building = Buildings[i];
				if (Building)
				{
					//Check if the building is a inventory building
					if (!Building->IsA(AInventoryTile::StaticClass()))
					{
						continue;
					}

					if (Building->isHaulerComming())
					{
						//Ignore buildings that already have a hauler comming
						continue;
					}

					if (!InInfluenceRange(Building->getTileX(), Building->getTileY(), Building->getBuildingSize()))
					{
						//Do not care about buildings that are outside your range
						continue;
					}

					AInventoryTile* InventoryTile = Cast<AInventoryTile>(Building);
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

							if (!Tile->IsA(UDirtRoadTile::StaticClass()))
							{
								continue;
							}

							if (Tile->IsBlocked())
							{
								continue;
							}

							Tile->DebugHighlightTile(1.0f);

							for (int32 k = 0; k < TilesAroundUs.Num(); k++)
							{
								UTileData* OurTile = TilesAroundUs[k];
								if (OurTile)
								{
									if (!OurTile->IsA(UDirtRoadTile::StaticClass()))
									{
										continue;
									}

									if (OurTile->IsBlocked())
									{
										continue;
									}

									OurTile->DebugHighlightTile(1.0f);
									TArray<UTileData*> Path = getGamemode()->getTable()->FindPathRoad(Tile, OurTile, false);
									if (Path.Num() > 0)
									{
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
