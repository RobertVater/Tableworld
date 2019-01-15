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

void ATableGamemode::BeginPlay()
{
	Super::BeginPlay();

	SetCurrentAge(StartAge);
	CurrentFunds = StartFunds;

	ModifyRescource(EItem::WoodLog, 25);

	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(this, ATableWorldTable::StaticClass(), OutActors);
	if(OutActors.IsValidIndex(0))
	{
		Table = Cast<ATableWorldTable>(OutActors[0]);
	}
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
			StoredRescources.Remove(Item);
			Event_StoredItemsUpdated.Broadcast();
			DebugWarning("Remove Rescource");

			return;
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
			SelectedBuilding = getGameInstance()->getBuilding(SelectedBuildingID);
			if(SelectedBuilding.ID != NAME_None)
			{
				if(getPlayerPawn())
				{
					getPlayerPawn()->SetCurrentBuilding(SelectedBuilding);
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

void ATableGamemode::SetTile(int32 X, int32 Y, ETileType type, bool bUpdateTexture)
{
	if(getTable())
	{
		getTable()->SetTile(X, Y, type,bUpdateTexture);
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
	for(int32 i = 0; i < Items.Num(); i++)
	{
		FNeededItems Item = Items[i];
		if(Item.NeededItem != EItem::None && Item.NeededItem != EItem::Max)
		{
			bool bFound = false;
			int32 Amount = getStoredItemAmount(Item.NeededItem, bFound);

			if(!bFound)
			{
				return false;
			}

			if(Amount < Item.NeededAmount)
			{
				return false;
			}

			return true;
		}
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
