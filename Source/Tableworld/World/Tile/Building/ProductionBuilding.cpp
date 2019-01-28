// Copyright by Robert Vater (Gunschlinger)

#include "ProductionBuilding.h"
#include "CityCentreTile.h"
#include "World/TableWorldTable.h"
#include "Creature/HaulerCreature.h"
#include "../TileData.h"
#include "Core/TableGamemode.h"

AProductionBuilding::AProductionBuilding()
{
	Tags.Add("ProductionBuilding");
}

void AProductionBuilding::StartWork()
{
	Super::StartWork();
	DebugLog("Start Work");


	//Check if we have enough items to produce our output
	GetWorldTimerManager().SetTimer(RescourceCheckTimer, this, &AProductionBuilding::TryProduceOutput, RescourceCheckTime);
}

void AProductionBuilding::StopWork()
{
	Super::StopWork();
	DebugLog("Stop Work");

	GetWorldTimerManager().ClearTimer(ProductionTimer);
	GetWorldTimerManager().ClearTimer(RescourceCheckTimer);

	//Call back all haulers
	for(int32 i = 0; i < Haulers.Num(); i++)
	{
		AHaulerCreature* Hauler = Haulers[i];
		if(Hauler)
		{
			Hauler->StopMovement();
			Hauler->ForceReturnJob();
		}
	}
}

void AProductionBuilding::TryProduceOutput()
{
	DebugLog("TryProduceOutput");

	//Check if we can even create a worker
	if (Haulers.Num() < MaxHaulers) 
	{
		if (getTable())
		{
			//Regen the tiles around us
			getTilesAroundUs(true);

			//First of all check if we are connected to a road.
			if (isConnectedToRoad())
			{
				//First check our last valid inventory to see if it still has the needed items
				if (LastValidInventory)
				{
					UTileData* StartTile = nullptr;
					UTileData* EndTile = nullptr;
					if (CheckIfValidInventory(LastValidInventory, StartTile, EndTile))
					{
						SendHaulerToInventory(LastValidInventory,StartTile , EndTile);
						return;
					}
				}

				//Check all nearby village centres if they have our input in stock
				TArray<ACityCentreTile*> Inventories = getTable()->getCityCentres();
				for (int32 i = 0; i < Inventories.Num(); i++)
				{
					ACityCentreTile* Inventory = Inventories[i];
					if (Inventory)
					{
						UTileData* StartTile = nullptr;
						UTileData* EndTile = nullptr;
						if (CheckIfValidInventory(Inventory, StartTile, EndTile))
						{
							//Update the last inventory to be this one
							LastValidInventory = Inventory;

							//Send a hauler to pickup the items.
							SendHaulerToInventory(Inventory, StartTile, EndTile);

							//We found a good inventory.
							return;
						}
					}
				}
			}
		}
	}

	GetWorldTimerManager().SetTimer(RescourceCheckTimer, this, &AProductionBuilding::TryProduceOutput, RescourceCheckTime);
}

void AProductionBuilding::ModifyInventory(EItem Item, int32 Amount, TMap<EItem, int32>& Storage)
{
	if (Item == EItem::None && Item == EItem::Max)return;

	if (!Storage.Contains(Item))
	{
		Storage.Add(Item, Amount);
		return;
	}

	int32 OldAmount = Storage.FindRef(Item) + Amount;

	if (OldAmount <= 0)
	{
		Storage.Remove(Item);
		return;
	}

	Storage.Emplace(Item, OldAmount);
}

bool AProductionBuilding::CheckIfValidInventory(ACityCentreTile* InInventory, UTileData*& StartTile, UTileData*& EndTile)
{
	if (InInventory)
	{
		TArray<UTileData*> InventoryTiles = InInventory->getTilesAroundUs(true);

		//Check if the inventory is connected to a road
		if (!InInventory->isConnectedToRoad())
		{
			return false;
		}

		if(!InInventory->ReserveItems(InputItems, this))
		{
			return false;
		}

		//Check if we are somehow connected to the inventory.
		for (int32 i = 0; i < getTilesAroundUs(false).Num(); i++)
		{
			UTileData* Tile = getTilesAroundUs(false)[i];

			if (Tile)
			{
				if (Tile->getTileType() == ETileType::DirtRoad)
				{
					//Check all tiles around the inventory
					for (int32 j = 0; j < InventoryTiles.Num(); j++)
					{
						UTileData* InventoryTile = InventoryTiles[j];
						if (InventoryTile) 
						{
							TArray<UTileData*> Path = getTable()->FindPathRoad(Tile, InventoryTile, false);
							if(Path.Num() > 0)
							{
								StartTile = Tile;
								EndTile = InventoryTile;
								
								//We found a valid path. This inventory is good enough
								return true;
							}
						}
					}
				}
			}
		}
	}

	return false;
}

void AProductionBuilding::SendHaulerToInventory(ACityCentreTile* InInventory, UTileData* StartTile, UTileData* EndTile)
{
	if (StartTile && EndTile) 
	{
		//Create a hauler
		if (HaulerClass)
		{
			AHaulerCreature* Hauler = GetWorld()->SpawnActor<AHaulerCreature>(HaulerClass, StartTile->getWorldCenter(), FRotator::ZeroRotator);
			if (Hauler)
			{
				Hauler->GiveProductionHaulJob(InInventory, this, EndTile, StartTile);

				//Bind Events
				Hauler->Event_HaulerReachedTarget.AddDynamic(this, &AProductionBuilding::OnHaulerReachTarget);
				Hauler->Event_HaulerReturnedHome.AddDynamic(this, &AProductionBuilding::OnHaulerReturnHome);

				Haulers.Add(Hauler);
				return;
			}
		}
	}

	DebugError("Couldnt create a hauler for " + GetName() + " (No Hauler class ? Start and EndTiles null ?)");
}

void AProductionBuilding::OnHaulerReachTarget(AHaulerCreature* Hauler)
{
	DebugError("reach target");
	
	//We reached the target
	if(Hauler)
	{
		DebugError("hauler");

		ACityCentreTile* Inventory = Hauler->getInventoryBuilding();
		if(Inventory)
		{
			DebugError("inv");

			//Check if the building still has the items we want
			if(Inventory->HasItems(InputItems, true))
			{
				for(auto Elem : InputItems)
				{
					EItem Item = Elem.Key;
					int32 Amount = Elem.Value;

					//Take away the items
					Inventory->ModifyInventory(Item, -Amount);

					if(getGamemode())
					{
						getGamemode()->ModifyRescource(Item, -Amount);
					}

					//Clear the reserve items we might have
					Inventory->ClearReserveItems(this);

					//Add the item to the hauler inventory
					Hauler->AddHaulItems(Item, Amount);
				}

				DebugError("return");
				Hauler->GiveReturnJob();
			}
			else
			{
				//The building is missing the items we need.

				//TODO THROW A NOTIFICATION
				DebugError("A Productionhauler was unable to get the items he needs!");

				Hauler->GiveReturnJob();
			}
		}else
		{
			DebugError("FAIL");
		}
	}
}

void AProductionBuilding::OnHaulerReturnHome(AHaulerCreature* Hauler)
{
	if(Hauler)
	{
		if (Hauler->HasItems())
		{
			TMap<EItem, int32> HaulerInventory = Hauler->getCarriedItems();
			for (auto Elem : HaulerInventory)
			{
				EItem Item = Elem.Key;
				int32 Amount = Elem.Value;

				ModifyInventory(Item, Amount, InputStorage);
			}

			//Try to produce the item
			ProduceItems();
		}

		Haulers.Remove(Hauler);
		Hauler->Destroy();
	}
}

void AProductionBuilding::ProduceItems()
{
	//Produce the output
	GetWorldTimerManager().SetTimer(ProductionTimer, this, &AProductionBuilding::OnItemProduce, ProductionTime);

	//Todo play work animation of the building.
}

void AProductionBuilding::OnItemProduce()
{
	//Add the output items to our storage and mark this building as haulable.
	ModifyInventory(OutputItem, OutputItemAmount, OutputStorage);

	CurrentInventory += OutputItemAmount;
	DebugError("Produced Item");

	GetWorldTimerManager().SetTimer(RescourceCheckTimer, this, &AProductionBuilding::TryProduceOutput, RescourceCheckTime);
}

EItem AProductionBuilding::getItemType()
{
	return OutputItem;
}

TArray<FProductionItem> AProductionBuilding::getInputItems()
{
	return InputItemsData;
}

TArray<FProductionItem> AProductionBuilding::getOutputItems()
{
	return OutputItemsData;
}

float AProductionBuilding::getProductionProgress()
{
	if(GetWorldTimerManager().IsTimerActive(ProductionTimer))
	{
		return GetWorldTimerManager().GetTimerElapsed(ProductionTimer) / ProductionTime;
	}

	return 0.0f;
}
