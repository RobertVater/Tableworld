// Fill out your copyright notice in the Description page of Project Settings.

#include "TableGamemode.h"
#include "World/TableWorldTable.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/TableHelper.h"
#include "Player/TablePlayerController.h"
#include "World/Tile/TileData.h"
#include "World/TableChunk.h"
#include "TableGameInstance.h"
#include "Player/TablePlayerPawn.h"
#include "World/Tile/Building/CityCentreBuilding.h"
#include "Savegame/TableSavegame.h"
#include "Interface/SaveLoadInterface.h"
#include "World/Tile/Building/HarvesterBuilding.h"
#include "World/Tile/Building/ProductionBuilding.h"

void ATableGamemode::BeginPlay()
{
	Super::BeginPlay();

	if (getGameInstance())
	{
		TArray<AActor*> OutActors;
		UGameplayStatics::GetAllActorsOfClass(this, ATableWorldTable::StaticClass(), OutActors);
		if (OutActors.IsValidIndex(0))
		{
			Table = Cast<ATableWorldTable>(OutActors[0]);
			if (Table)
			{
				if (!getGameInstance()->isLoadGame())
				{
					//Start a new game.
					DebugWarning("Start new game");

					//Get the Seed
					WorldSeed = getGameInstance()->getSeed();
					FMath::RandInit(getGameInstance()->getSeed());
					SetCurrentAge(StartAge);

					ModifyRescource(EItem::WoodLog, 25);
					ModifyRescource(EItem::Stone, 6);

					Table->InitTable(getGameInstance()->getSeed(), getGameInstance()->getWorldSize(), getGameInstance()->hasRivers(), getGameInstance()->getRiverCount());

					//Reste world settings.
					getGameInstance()->ClearWorldSettings();
				}else
				{
					//Load a save
					LoadGame(getGameInstance()->getSaveGame());

					getGameInstance()->ClearLoadGame();
					return;
				}
			}
		}
	}
}

void ATableGamemode::ModifyTime(int32 NewTime)
{
	DebugError("ModifyTime " + FString::FromInt(NewTime));
	
	UGameplayStatics::SetGamePaused(this, false);
	UGameplayStatics::SetGlobalTimeDilation(this, NewTime);

	GameSpeed = NewTime;
	Event_GameSpeedUpdated.Broadcast(NewTime);
}

bool ATableGamemode::StopTime()
{
	DebugError("StopTime");
	bGamePaused = !bGamePaused;
	
	UGameplayStatics::SetGamePaused(this, bGamePaused);

	if(bGamePaused)
	{
		Event_GameSpeedUpdated.Broadcast(0);
	}

	return bGamePaused;
}

void ATableGamemode::ResetTime()
{
	ModifyTime(GameSpeed);
}

void ATableGamemode::SaveGame(FString SaveName)
{
	
}

void ATableGamemode::LoadGame(FString SaveName)
{
	DebugLog("Trying to load " + SaveName);

	UTableSavegame* Save = Cast<UTableSavegame>(UGameplayStatics::LoadGameFromSlot(SaveName, 0));
	if(Save)
	{
		
	}else
	{
		DebugError("Failed to load " + SaveName);
	}
}

void ATableGamemode::AddNotification(FTableNotification NewNotifi)
{
	Event_NewNotification.Broadcast(NewNotifi);
}

void ATableGamemode::AddFloatingItem(EItem item, int32 Amount, FVector WorldLoc)
{
	Event_FloatingItem.Broadcast(item, Amount, WorldLoc);
}

void ATableGamemode::ModifyRescource(EItem Item, int32 AddAmount)
{
	DebugWarning("Modify Rescource " + FString::FromInt(AddAmount));

	if (Item == EItem::None && Item == EItem::Max)return;
	
	if(StoredRescources.Contains(Item))
	{
		int32 Stored = StoredRescources.FindRef(Item);
		Stored += AddAmount;

		//Check if we should remove the Item
		if(Stored <= 0)
		{
			Stored = 0;
		}

		//Otherwise update
		StoredRescources.Emplace(Item, Stored);
		Event_StoredItemsUpdated.Broadcast();
		DebugWarning("Update Rescource");

		return;
	}
	
	if(AddAmount > 0)
	{
		//Add the item
		StoredRescources.Add(Item, AddAmount);
		Event_StoredItemsUpdated.Broadcast();
		DebugWarning("Add Rescource");
	}
}

bool ATableGamemode::ConsumeRescource(TArray<FNeededItems> Rescources)
{
	DebugWarning("Consume-----------------");
	if(getTable())
	{
		//A list of storage buildings that own the required items
		TArray<ACityCentreTile*> FoundStorage;
		
		TArray<ACityCentreTile*> Storage = getTable()->getCityCentres();
		for (int32 j = 0; j < Rescources.Num(); j++)
		{
			FNeededItems Needed = Rescources[j];
			int32 NeededAmount = Needed.NeededAmount;

			ModifyRescource(Needed.NeededItem, -Needed.NeededAmount);
			for (int32 i = 0; i < Storage.Num(); i++)
			{
				ACityCentreTile* Building = Storage[i];
				if (Building)
				{
					DebugWarning("New Building-----------------");
					TMap<EItem, int32> Stored = Building->getStoredItems();
					for (auto Elem : Stored)
					{
						EItem Item = Elem.Key;
						int32 Stored = Elem.Value;

						if(Item == Needed.NeededItem)
						{
							DebugWarning("Store " + FString::FromInt(Stored));
							
							if(Stored >= NeededAmount)
							{
								Building->ModifyInventory(Item, -NeededAmount);
								NeededAmount = 0;
							}else
							{
								if(Stored < NeededAmount)
								{
									int32 Dif = NeededAmount - Stored;
									Building->ModifyInventory(Item, -Dif);

									NeededAmount -= Stored;
								}
							}

							if(NeededAmount <= 0)
							{
								break;
							}
						}
					}
				}
			}
		}
	}

	return true;
}

void ATableGamemode::SetCurrentAge(ETableAge nCurrentAge)
{
	CurrentAge = nCurrentAge;
}

void ATableGamemode::SelectBuilding(FName SelectedBuildingID)
{
	if(SelectedBuildingID != NAME_None)
	{
		if(getGameInstance())
		{
			bool bFound = false;
			SelectedBuilding = getGameInstance()->getBuilding(SelectedBuildingID, bFound);

			if (bFound) 
			{
				if (SelectedBuilding.ID != NAME_None)
				{
					if (getPlayerPawn())
					{
						getPlayerPawn()->SetCurrentBuilding(SelectedBuilding);
					}
				}
			}
		}
	}
}

ATablePlayerController* ATableGamemode::getPlayerController()
{
	if(!PC)
	{
		PC = Cast<ATablePlayerController>(GetWorld()->GetFirstPlayerController());
	}

	return PC;
}

ATablePlayerPawn* ATableGamemode::getPlayerPawn()
{
	if(!Player)
	{
		if(getPlayerController())
		{
			Player = Cast<ATablePlayerPawn>(getPlayerController()->GetPawn());
		}
	}

	return Player;
}

UTableGameInstance* ATableGamemode::getGameInstance()
{
	if(!GI)
	{
		GI = Cast<UTableGameInstance>(GetGameInstance());
	}

	return GI;
}

ATableWorldTable* ATableGamemode::getTable()
{
	return Table;
}

ETableAge ATableGamemode::getCurrentAge()
{
	return CurrentAge;
}

void ATableGamemode::SetTile(int32 X, int32 Y, ETileType type, bool bUpdateTexture, bool bModifyTile)
{
	if(getTable())
	{
		getTable()->SetTile(X, Y, type,bUpdateTexture, bModifyTile);
	}
}

ATableChunk* ATableGamemode::getChunkForTile(int32 X, int32 Y)
{
	if(getTable())
	{
		return getTable()->getChunkForTile(X, Y);
	}

	return nullptr;
}

UTileData* ATableGamemode::getTile(int32 X, int32 Y)
{
	if(getTable())
	{
		return getTable()->getTile(X, Y);
	}

	return nullptr;
}

FTableBuilding ATableGamemode::getSelectedBuilding()
{
	return SelectedBuilding;
}

bool ATableGamemode::OwnNeededItems(TArray<FNeededItems> Items)
{
	if (Items.Num() > 0) 
	{
		for (int32 i = 0; i < Items.Num(); i++)
		{
			FNeededItems Item = Items[i];
			if (Item.NeededItem != EItem::None && Item.NeededItem != EItem::Max)
			{
				bool bFound = false;
				int32 Amount = getStoredItemAmount(Item.NeededItem, bFound);

				if (!bFound)
				{
					return false;
				}

				if (Amount < Item.NeededAmount)
				{
					return false;
				}
			}
		}

		return true;
	}

	return false;
}

TMap<EItem, int32> ATableGamemode::getStoredItems()
{
	return StoredRescources;
}

int32 ATableGamemode::getStoredItemAmount(EItem Item, bool& bFound)
{
	if(StoredRescources.Contains(Item))
	{
		bFound = true;
		return StoredRescources.FindRef(Item);
	}

	bFound = false;
	return 0;
}

int32 ATableGamemode::getGameTime()
{
	return GameSpeed;
}

int32 ATableGamemode::getWorldSeed()
{
	return WorldSeed;
}
