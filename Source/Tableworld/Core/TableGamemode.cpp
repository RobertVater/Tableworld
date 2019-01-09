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

	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(this, ATableWorldTable::StaticClass(), OutActors);
	if(OutActors.IsValidIndex(0))
	{
		Table = Cast<ATableWorldTable>(OutActors[0]);
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
