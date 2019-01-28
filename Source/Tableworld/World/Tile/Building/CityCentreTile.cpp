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

void ACityCentreTile::Place(FVector PlaceLoc, TArray<FVector2D> nPlacedOnTiles, FTableBuilding nBuildingData, bool bNewRotated)
{
	Super::Place(PlaceLoc, nPlacedOnTiles, nBuildingData, bNewRotated);

	StartWork();
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

bool ACityCentreTile::InInfluenceRange(int32 X, int32 Y, FVector2D Size)
{
	if(getTable())
	{
		bool bInRange = true;

		int32 Radius = getBuildGridRadius();
		for (int32 x = -Radius; x <= Radius; x++)
		{
			for (int32 y = -Radius; y <= Radius; y++)
			{
				if ((x * x) + (y * y) <= (Radius * Radius))
				{
					int32 XX = (GetActorLocation().X / 100 + x);
					int32 YY = (GetActorLocation().Y / 100 + y);

					for (int32 bx = 0; bx < Size.X; bx++) 
					{
						for (int32 by = 0; by < Size.Y; by++) 
						{
							int32 Distance = UTableHelper::getDistance(getCenterX(), getCenterY(), X + bx, Y + by);
							if (Distance >= Radius)
							{
								bInRange = false;
							}
						}
					}
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
			EItem HaulItem = EItem::None;
			int32 HaulAmount = 0;

			if (nHauler->getItemZero(HaulItem, HaulerAmount))
			{
				DebugWarning("Haul completed " + FString::FromInt(HaulAmount));

				getGamemode()->AddFloatingItem(HaulItem, HaulerAmount, getWorldCenter());

				//Add to local inventory
				ModifyInventory(HaulItem, HaulerAmount);

				//Add the item to the global inventory
				getGamemode()->ModifyRescource(HaulItem, HaulerAmount);
			}
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

bool ACityCentreTile::ReserveItems(TMap<EItem, int32> Items, ABuildableTile* Building)
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
			ReservedItem.ReservingBuilding = Building;

			ReservedItems.Add(ReservedItem);
		}

		return true;
	}

	return false;
}

void ACityCentreTile::ClearReserveItems(ABuildableTile* Owner)
{
	if (Owner)
	{
		for (int32 i = 0; i < ReservedItems.Num(); i++)
		{
			FReservedItem Item = ReservedItems[i];
			if(Item.ReservingBuilding == Owner)
			{
				ReservedItems.RemoveAt(i);
			}
		}
	}
}

AInventoryTile* ACityCentreTile::getValidHaulGoal(FVector2D& InTile, FVector2D& OutTile)
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

			TArray<ABuildableTile*> Buildings = getGamemode()->getTable()->getBuildings();
			for (int32 i = 0; i < Buildings.Num(); i++)
			{
				ABuildableTile* Building = Buildings[i];
				if (Building)
				{
					Building->getTilesAroundUs(true);
					
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

					//Check if the target building is connected to a road
					if(!Building->isConnectedToRoad())
					{
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
