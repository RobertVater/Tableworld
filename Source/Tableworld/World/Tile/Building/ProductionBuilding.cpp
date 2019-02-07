// Copyright by Robert Vater (Gunschlinger)

#include "ProductionBuilding.h"
#include "CityCentreTile.h"
#include "World/TableWorldTable.h"
#include "Creature/HaulerCreature.h"
#include "../TileData.h"
#include "Core/TableGamemode.h"
#include "Misc/TableHelper.h"

AProductionBuilding::AProductionBuilding()
{
	Tags.Add("ProductionBuilding");
}

void AProductionBuilding::Place(FVector PlaceLoc, TArray<FVector2D> nPlacedOnTiles, FTableBuilding nBuildingData, bool bRotated, bool bLoadBuilding)
{
	Super::Place(PlaceLoc, nPlacedOnTiles, nBuildingData, bRotated, bLoadBuilding);

	if (!bLoadBuilding) 
	{
		StartWork();

		//Spawn all haulers we can
		SpawnHaulers();
	}
}

void AProductionBuilding::StartWork()
{
	Super::StartWork();

	//Check if we have enough items to produce our output
	GetWorldTimerManager().SetTimer(RescourceCheckTimer, this, &AProductionBuilding::TryProduceOutput, RescourceCheckTime);
}

void AProductionBuilding::StopWork()
{
	Super::StopWork();

	GetWorldTimerManager().ClearTimer(ProductionTimer);
	GetWorldTimerManager().ClearTimer(RescourceCheckTimer);

	//Call back all haulers
	for(int32 i = 0; i < Haulers.Num(); i++)
	{
		AHaulerCreature* Hauler = Haulers[i];
		if(Hauler)
		{
			Hauler->ForceReturnJob();
		}
	}
}

void AProductionBuilding::TryProduceOutput()
{
	DebugError("Try");
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
					SendHaulerToInventory(LastValidInventory, StartTile, EndTile);
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

	GetWorldTimerManager().SetTimer(RescourceCheckTimer, this, &AProductionBuilding::TryProduceOutput, RescourceCheckTime);
}

void AProductionBuilding::SpawnHaulers()
{
	for (int32 i = 0; i < MaxHaulers; i++)
	{
		AHaulerCreature* Hauler = SpawnHauler(getWorldCenter());
		if (Hauler)
		{
			Hauler->DeactivateCreature();

			Haulers.Add(Hauler);
		}
	}
}

AHaulerCreature* AProductionBuilding::SpawnHauler(FVector Location)
{
	AHaulerCreature* Hauler = GetWorld()->SpawnActor<AHaulerCreature>(HaulerClass, Location, FRotator::ZeroRotator);
	if(Hauler)
	{
		//Bind Events
		Hauler->Event_HaulerReachedTarget.AddDynamic(this, &AProductionBuilding::OnHaulerReachTarget);
		Hauler->Event_HaulerReturnedHome.AddDynamic(this, &AProductionBuilding::OnHaulerReturnHome);
	}

	return Hauler;
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

		if(!InInventory->ReserveItems(InputItems, UID))
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
	DebugError("Send Hauler");
	if (StartTile && EndTile) 
	{
		StartTile->DebugHighlightTile(10, FColor::Green);
		EndTile->DebugHighlightTile(10, FColor::Red);
		
		AHaulerCreature* Hauler = nullptr;

		//Check if we got some deactivated haulers
		for (int32 i = 0; i < Haulers.Num(); i++)
		{
			AHaulerCreature* FoundHauler = Haulers[i];
			if (FoundHauler)
			{
				if (FoundHauler->getStatus() == ECreatureStatus::Deactivated)
				{
					FoundHauler->ActivateCreature();
					Hauler = FoundHauler;
				}
			}
		}

		if (Hauler)
		{
			Hauler->SetActorLocation(StartTile->getWorldCenter());
			Hauler->GiveHaulJob(InInventory->getUID(), getUID(), EndTile, StartTile);
		}
	}
}

void AProductionBuilding::OnHaulerReachTarget(AHaulerCreature* Hauler)
{
	DebugError("OnHaulerReachTarget");
	
	//We reached the target
	if(Hauler)
	{
		ACityCentreTile* Inventory = Cast<ACityCentreTile>(getTable()->getBuildingWithID(Hauler->getTargetBuildingUID()));
		if(Inventory)
		{
			//Check if the building still has the items we want
			if(Inventory->HasItems(InputItems, true))
			{
				DebugError("-----------------------------------------" + FString::FromInt(InputItems.Num()));
				for(auto Elem : InputItems)
				{
					DebugError("Elem Called");
					
					EItem Item = Elem.Key;
					int32 Amount = Elem.Value;

					//Take away the items
					Inventory->ModifyInventory(Item, -Amount);

					if(getGamemode())
					{
						getGamemode()->ModifyRescource(Item, -Amount);
					}

					//Clear the reserve items we might have
					Inventory->ClearReserveItems(getUID());

					//Add the item to the hauler inventory
					Hauler->AddHaulItems(Item, Amount);
				}

				Hauler->GiveReturnJob();
			}
			else
			{
				//The building is missing the items we need.
				if(getGamemode())
				{
					FTableNotification NewNotify;
					NewNotify.NotificationType = ETableNotificationType::Problem;
					NewNotify.NotificationTitle = FText::AsCultureInvariant("Problem");
					NewNotify.NotificationText = FText::AsCultureInvariant("A Productionhauler was unable to get the items he needs!");
					NewNotify.bForcePause = false;
					NewNotify.NotificationLocation = Hauler->GetActorLocation();
					getGamemode()->AddNotification(NewNotify);
				}
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

		Hauler->ClearInventory();
		Hauler->DeactivateCreature();
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
	LoadProgress = 0.0f;
	
	//Add the output items to our storage and mark this building as haulable.
	ModifyInventory(OutputItem, OutputItemAmount, OutputStorage);

	CurrentInventory += OutputItemAmount;

	if (getGamemode())
	{
		getGamemode()->AddFloatingItem(OutputItem, OutputItemAmount, getWorldCenter());

		if(CurrentInventory >= InventorySize)
		{
			FTableNotification NewNotify;
			NewNotify.NotificationType = ETableNotificationType::Problem;
			NewNotify.NotificationTitle = FText::AsCultureInvariant("Full Storage");
			NewNotify.NotificationText = FText::AsCultureInvariant("A Production building has stopped working because its storage is full!");
			NewNotify.bForcePause = false;
			NewNotify.NotificationLocation = getWorldCenter();

			getGamemode()->AddNotification(NewNotify);
		}
	}
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
		return (GetWorldTimerManager().GetTimerElapsed(ProductionTimer) + LoadProgress) / ProductionTime;
	}

	return 0.0f;
}

void AProductionBuilding::LoadData(FTableSaveProductionBuilding Data)
{
	UID = Data.UID;
	CurrentInventory = Data.CurrentInventory;
	InputStorage = Data.InputStorage;
	OutputStorage = Data.OutputStorage;

	if(Data.ProductionTimer > 0)
	{
		DebugError("Start Timer");
		LoadProgress = (ProductionTime - Data.ProductionTimer);

		GetWorldTimerManager().SetTimer(ProductionTimer, this, &AProductionBuilding::OnItemProduce, Data.ProductionTimer);
	}

	if(Data.ResCheckTimer > 0)
	{
		GetWorldTimerManager().SetTimer(RescourceCheckTimer, this, &AProductionBuilding::TryProduceOutput, Data.ResCheckTimer);
	}else
	{
		//Start the default Timer
		GetWorldTimerManager().SetTimer(RescourceCheckTimer, this, &AProductionBuilding::TryProduceOutput, RescourceCheckTime);
	}

	if(Data.LastValidInventoryUID != NAME_None)
	{
		// get ref to the building with the specified ID
		if (getTable()) 
		{
			LastValidInventory = Cast<ACityCentreTile>(getTable()->getBuildingWithID(Data.LastValidInventoryUID));
		}
	}

	SpawnHaulers();
	for (int32 i = 0; i < Data.Workers.Num(); i++)
	{
		FTableSaveHaulerCreature WorkerData = Data.Workers[i];

		if (Haulers.IsValidIndex(i)) 
		{
			AHaulerCreature* Hauler = Haulers[i];
			if(Hauler)
			{
				Hauler->LoadData(WorkerData);
				Hauler->SetActorLocation(WorkerData.Location);
			}
		}
	}
}

void AProductionBuilding::SaveData_Implementation(UTableSavegame* Savegame)
{
	if(Savegame)
	{
		FTableSaveProductionBuilding Produciton;
		Produciton.BuildingID = getBuildingData().ID;
		Produciton.UID = UID;
		Produciton.TileX = getTileX();
		Produciton.TileY = getTileY();
		Produciton.bRotated = bRotated;
		Produciton.Rotation = GetActorRotation().Yaw;
		Produciton.CurrentInventory = CurrentInventory;
		Produciton.InputStorage = InputStorage;
		Produciton.OutputStorage = OutputStorage;
		Produciton.ProductionTimer = GetWorldTimerManager().GetTimerElapsed(ProductionTimer);
		Produciton.ResCheckTimer = GetWorldTimerManager().GetTimerRemaining(RescourceCheckTimer);

		if (LastValidInventory)
		{
			Produciton.LastValidInventoryUID = LastValidInventory->getUID();
		}

		//Save Workers
		for (int32 i = 0; i < Haulers.Num(); i++)
		{
			AHaulerCreature* Worker = Haulers[i];
			if (Worker)
			{
				FTableSaveHaulerCreature WorkerData = Worker->getSaveData();
				Produciton.Workers.Add(WorkerData);
			}
		}

		Savegame->SavedProduction.Add(Produciton);
	}
}
