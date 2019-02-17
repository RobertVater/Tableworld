// Fill out your copyright notice in the Description page of Project Settings.

#include "TableGameInstance.h"
#include "Misc/TableHelper.h"

FTableBuilding UTableGameInstance::getBuilding_Implementation(const FName& ID, bool& bFound)
{
	return FTableBuilding();
}

TArray<FTableBuilding> UTableGameInstance::getBuildingsForAgeAndCategory_Implementation(ETableAge Age, ETableTileCategory Category, bool& bFound)
{
	return TArray<FTableBuilding>();
}

FTableItem UTableGameInstance::getItem_Implementation(EItem Item, bool& bFound)
{
	return FTableItem();
}

FTableRescource UTableGameInstance::getRescource_Implementation(ETileRescources Rescource, bool& bFound)
{
	return FTableRescource();
}

void UTableGameInstance::Init()
{
	Super::Init();

	LoadHumanNames();
}

void UTableGameInstance::LoadHumanNames()
{
	Human_Names.Empty();

	FString SavegamePath = FString(FPaths::GameContentDir() + "Names/" + "Human_Names" + ".txt");
	DebugWarning(SavegamePath);

	TArray<FString> RawNames;
	FFileHelper::LoadFileToStringArray(RawNames, *SavegamePath);

	for(int32 i = 0; i < RawNames.Num(); i++)
	{
		Human_Names.Add(FName(*RawNames[i]));
	}

	DebugLog("-Loaded " + FString::FromInt(Human_Names.Num()) + " Humannames!");
}

void UTableGameInstance::PrepareNewGame(int32 nSeed, uint8 nWorldSize, bool nRiver, uint8 nRiverCount)
{
	Seed = nSeed;
	WorldSize = nWorldSize;
	bHasRiver = nRiver;
	RiverCount = nRiverCount;
}

void UTableGameInstance::PrepareLoadGame(FString nSaveGame)
{
	bLoadGame = true;
	SaveGame = nSaveGame;
}

void UTableGameInstance::ClearLoadGame()
{
	bLoadGame = false;
	SaveGame = "";

	ClearWorldSettings();
}

void UTableGameInstance::ClearWorldSettings()
{
	Seed = 0;
	WorldSize = 0;
	bHasRiver = false;
	RiverCount = 0;
}

void UTableGameInstance::ShowLoadingScreen()
{
	Event_ShowLoadingScreen.Broadcast();
}

void UTableGameInstance::HideLoadingScreen()
{
	Event_HideLoadingScreen.Broadcast();
}

void UTableGameInstance::ShowError(FText Title, FText Text)
{
	Event_ShowError.Broadcast(Title, Text);
}

void UTableGameInstance::ShowDialog(FText Title, FText Text, FText ChoiceA, FText ChoiceB)
{
	Event_ShowDialog.Broadcast(Title, Text, ChoiceA, ChoiceB);
}

bool UTableGameInstance::isLoadGame()
{
	return bLoadGame;
}

FString UTableGameInstance::getSaveGame()
{
	return SaveGame;
}

int32 UTableGameInstance::getSeed()
{
	return Seed;
}

uint8 UTableGameInstance::getWorldSize()
{
	return WorldSize;
}

bool UTableGameInstance::hasRivers()
{
	return bHasRiver;
}

uint8 UTableGameInstance::getRiverCount()
{
	return RiverCount;
}

FName UTableGameInstance::getHumanName(int32 Index)
{
	if(Human_Names.IsValidIndex(Index))
	{
		return Human_Names[Index];
	}

	return FName();
}

int32 UTableGameInstance::getRandomHumanName()
{
	int32 RIndex = FMath::RandRange(0, Human_Names.Num() - 1);
	if(Human_Names.IsValidIndex(RIndex))
	{
		return RIndex;
	}

	return 0;
}
