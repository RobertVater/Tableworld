// Copyright by Robert Vater (Gunschlinger)

#include "FactoryBuilding.h"
#include "CityCentreBuilding.h"
#include "World/TableWorldTable.h"
#include "Creature/HaulerCreature.h"
#include "../TileData.h"
#include "Core/TableGamemode.h"
#include "Misc/TableHelper.h"

AFactoryBuilding::AFactoryBuilding()
{
	WorkerComponent = CreateDefaultSubobject<UWorkerComponent>(TEXT("WorkerComponent"));
	InputItemComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InputItemComponent"));
	
	Tags.Add("ProductionBuilding");
}

void AFactoryBuilding::Place(FVector PlaceLoc, TArray<FVector2D> nPlacedOnTiles, FTableBuilding nBuildingData, bool bRotated, bool bLoadBuilding)
{
	Super::Place(PlaceLoc, nPlacedOnTiles, nBuildingData, bRotated, bLoadBuilding);

	WorkerComponent->Init(this);

	if (!bLoadBuilding) 
	{
		StartWork();

		//Spawn all haulers we can
		WorkerComponent->SpawnAllWorkers();
	}

	TArray<AWorkerCreature*> Workers = getWorkers();
	for (int32 i = 0; i < Workers.Num(); i++)
	{
		AHaulerCreature* Hauler = Cast<AHaulerCreature>(Workers[i]);
		if (Hauler)
		{
			Hauler->Event_HaulerReachedTarget.AddDynamic(this, &AFactoryBuilding::OnHaulerReachTarget);
			Hauler->Event_HaulerReturnedHome.AddDynamic(this, &AFactoryBuilding::OnHaulerReturnHome);
		}
	}
}

void AFactoryBuilding::StartWork()
{
	Super::StartWork();

	//Check if we have enough items to produce our output
	GetWorldTimerManager().SetTimer(RescourceCheckTimer, this, &AFactoryBuilding::TryProduceOutput, RescourceCheckTime);
}

void AFactoryBuilding::StopWork()
{
	Super::StopWork();

	GetWorldTimerManager().ClearTimer(ProductionTimer);
	GetWorldTimerManager().ClearTimer(RescourceCheckTimer);

	//Call back all haulers
	TArray<AWorkerCreature*> Haulers = getWorkers();
	for(int32 i = 0; i < Haulers.Num(); i++)
	{
		AHaulerCreature* Hauler = Cast<AHaulerCreature>(Haulers[i]);
		if(Hauler)
		{
			Hauler->ForceReturnJob();
		}
	}
}

void AFactoryBuilding::TryProduceOutput()
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

	GetWorldTimerManager().SetTimer(RescourceCheckTimer, this, &AFactoryBuilding::TryProduceOutput, RescourceCheckTime);
}


void AFactoryBuilding::ModifyInventory(EItem Item, int32 Amount)
{
	InventoryComponent->ModifyInventory(Item, Amount);
}

bool AFactoryBuilding::CheckIfValidInventory(ACityCentreTile* InInventory, UTileData*& StartTile, UTileData*& EndTile)
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

void AFactoryBuilding::SendHaulerToInventory(ACityCentreTile* InInventory, UTileData* StartTile, UTileData* EndTile)
{
	DebugError("Send Hauler");
	if (StartTile && EndTile) 
	{
		StartTile->DebugHighlightTile(10, FColor::Green);
		EndTile->DebugHighlightTile(10, FColor::Red);
		
		AHaulerCreature* Hauler = nullptr;

		//Check if we got some deactivated haulers
		TArray<AWorkerCreature*> Haulers = getWorkers();
		for (int32 i = 0; i < Haulers.Num(); i++)
		{
			AHaulerCreature* FoundHauler = Cast<AHaulerCreature>(Haulers[i]);
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

void AFactoryBuilding::OnHaulerReachTarget(AHaulerCreature* Hauler)
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

void AFactoryBuilding::OnHaulerReturnHome(AHaulerCreature* Hauler)
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

				InputItemComponent->ModifyInventory(Item, Amount);
			}

			//Try to produce the item
			ProduceItems();
		}

		Hauler->ClearInventory();
		Hauler->DeactivateCreature();
	}
}

void AFactoryBuilding::ProduceItems()
{
	//Produce the output
	GetWorldTimerManager().SetTimer(ProductionTimer, this, &AFactoryBuilding::OnItemProduce, ProductionTime);

	//Todo play work animation of the building.
}

void AFactoryBuilding::OnItemProduce()
{
	LoadProgress = 0.0f;
	
	//Add the output items to our storage and mark this building as haulable.
	InventoryComponent->ModifyInventory(OutputItem, OutputItemAmount);

	//Remove the input items
	InputItemComponent->ClearInventory();

	if (getGamemode())
	{
		getGamemode()->AddFloatingItem(OutputItem, OutputItemAmount, getWorldCenter());

		if(getCurrentStorage() >= getMaxStorage())
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

	GetWorldTimerManager().SetTimer(RescourceCheckTimer, this, &AFactoryBuilding::TryProduceOutput, RescourceCheckTime);
}

TArray<AWorkerCreature*> AFactoryBuilding::getWorkers()
{
	return WorkerComponent->getWorkers();
}

EItem AFactoryBuilding::getItemType()
{
	return OutputItem;
}

TArray<FProductionItem> AFactoryBuilding::getInputItems()
{
	return InputItemsData;
}

TArray<FProductionItem> AFactoryBuilding::getOutputItems()
{
	return OutputItemsData;
}

float AFactoryBuilding::getProductionProgress()
{
	if(GetWorldTimerManager().IsTimerActive(ProductionTimer))
	{
		return (GetWorldTimerManager().GetTimerElapsed(ProductionTimer) + LoadProgress) / ProductionTime;
	}

	return 0.0f;
}


void AFactoryBuilding::SaveData_Implementation(UTableSavegame* Savegame)
{
	
}

FTableInfoPanel AFactoryBuilding::getInfoPanelData_Implementation()
{
	FTableInfoPanel Data = Super::getInfoPanelData_Implementation();
	Data.WorkerComponent = WorkerComponent;
	Data.InventoryComponent = InventoryComponent;

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

	//Add a progressbar
	FTableInfo_Progressbar Bar;
	Bar.Name = FText::FromString("Production: ");
	Bar.StartValue = (GetWorldTimerManager().GetTimerElapsed(ProductionTimer) + LoadProgress);
	Bar.MaxValue = ProductionTime;
	
	Data.Progressbar.Add(Bar);
	Data.PanelSize = FVector2D(350, 400);

	return Data;
}

FTableInfoPanel AFactoryBuilding::getUpdateInfoPanelData_Implementation()
{
	FTableInfoPanel Data = Super::getInfoPanelData_Implementation();

	FTableInfo_Progressbar Bar;
	Bar.StartValue = (GetWorldTimerManager().GetTimerElapsed(ProductionTimer) + LoadProgress);

	Data.Progressbar.Add(Bar);

	Data.InventoryComponent = InventoryComponent;

	return Data;
}
