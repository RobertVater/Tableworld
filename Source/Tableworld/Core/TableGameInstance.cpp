// Fill out your copyright notice in the Description page of Project Settings.

#include "TableGameInstance.h"
#include "Misc/TableHelper.h"
#include "UI/DialogChoice.h"
#include "UI/PopupInfo.h"
#include "Civilization/TableCivilization.h"
#include "Savegame/TableCivSavegame.h"

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

void UTableGameInstance::LoadCivilization(UTableCivSavegame* CopyData)
{
	if(!ActiveCivilization)
	{
		ActiveCivilization = NewObject<UTableCivilization>();
	}

	if(CopyData)
	{
		ActiveCivilization->CivName = FText::FromName(CopyData->Name);
		ActiveCivilization->CivTitle = FText::FromName(CopyData->Title);

		ActiveCivilization->TakenTraits = CopyData->TakenTraits;
	}
}

void UTableGameInstance::LoadHumanNames()
{
	HumanMale_Names.Empty();
	HumanFemale_Names.Empty();

	//Male
	FString MalePath = FString(FPaths::GameContentDir() + "Names/" + "HumanMale_Names" + ".txt");

	TArray<FString> RawMaleNames;
	FFileHelper::LoadFileToStringArray(RawMaleNames, *MalePath);

	for(int32 i = 0; i < RawMaleNames.Num(); i++)
	{
		HumanMale_Names.Add(FName(*RawMaleNames[i]));
	}

	//Female
	FString FemalePath = FString(FPaths::GameContentDir() + "Names/" + "HumanFemale_Names" + ".txt");

	TArray<FString> RawFemaleNames;
	FFileHelper::LoadFileToStringArray(RawFemaleNames, *FemalePath);

	for (int32 i = 0; i < RawFemaleNames.Num(); i++)
	{
		HumanFemale_Names.Add(FName(*RawFemaleNames[i]));
	}

	DebugLog("-Loaded " + FString::FromInt(HumanMale_Names.Num()) + " Human male names!");
	DebugLog("-Loaded " + FString::FromInt(HumanFemale_Names.Num()) + " Human female names!");
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

UDialogChoice* UTableGameInstance::CreateDialogChoice(FText Title, FText Text, FText A, FText B)
{
	if(DialogChoice_Class)
	{
		UDialogChoice* Popup = CreateWidget<UDialogChoice>(GetWorld()->GetFirstPlayerController(), DialogChoice_Class);
		if(Popup)
		{
			Popup->Set(Title, Text, A, B);
			Popup->AddToPlayerScreen(99999);

			return Popup;
		}
	}

	return nullptr;
}

UPopupInfo* UTableGameInstance::CreatePopupInfo(FText Title, FText Text)
{
	if (PopupInfo_Class)
	{
		UPopupInfo* Popup = CreateWidget<UPopupInfo>(GetWorld()->GetFirstPlayerController(), PopupInfo_Class);
		if (Popup)
		{
			Popup->Set(Title, Text);
			Popup->AddToPlayerScreen(99999);

			return Popup;
		}
	}

	return nullptr;
}

UUserWidget* UTableGameInstance::CreateLoadingScreen()
{
	DebugLog("Called");
	if(LoadingScreen_Class)
	{
		DebugLog("Class k");

		UUserWidget* Loading = CreateWidget<UUserWidget>(GetWorld()->GetFirstPlayerController(), LoadingScreen_Class);
		if(Loading)
		{
			Loading->AddToPlayerScreen(99998);
			DebugLog("Add Loadingscreen");

			return Loading;
		}
	}

	return nullptr;
}

void UTableGameInstance::ShowError(FText Title, FText Text)
{
	Event_ShowError.Broadcast(Title, Text);
}

void UTableGameInstance::ShowDialog(FText Title, FText Text, FText ChoiceA, FText ChoiceB)
{
	Event_ShowDialog.Broadcast(Title, Text, ChoiceA, ChoiceB);
}

FCivTrait UTableGameInstance::getTrait_Implementation(const FString& ID, bool& bFound)
{
	return FCivTrait();
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

FName UTableGameInstance::getHumanName(int32 Index, bool bIsMale)
{
	if (bIsMale)
	{
		if (HumanMale_Names.IsValidIndex(Index))
		{
			return HumanMale_Names[Index];
		}
	}

	if (HumanFemale_Names.IsValidIndex(Index))
	{
		return HumanFemale_Names[Index];
	}

	return FName();
}

int32 UTableGameInstance::getRandomHumanName(bool bIsMale)
{
	if (bIsMale) 
	{
		int32 RMaleIndex = FMath::RandRange(0, HumanMale_Names.Num() - 1);
		if (HumanMale_Names.IsValidIndex(RMaleIndex))
		{
			return RMaleIndex;
		}
	}

	int32 RFemaleIndex = FMath::RandRange(0, HumanFemale_Names.Num() - 1);
	if (HumanFemale_Names.IsValidIndex(RFemaleIndex))
	{
		return RFemaleIndex;
	}

	return 0;
}

FText UTableGameInstance::getCivName()
{
	if(ActiveCivilization)
	{
		return ActiveCivilization->CivName;
	}

	return FText();
}

FText UTableGameInstance::getCivTitle()
{
	if (ActiveCivilization)
	{
		return ActiveCivilization->CivTitle;
	}

	return FText();
}

TArray<FString> UTableGameInstance::getCivTraits()
{
	if (ActiveCivilization)
	{
		return ActiveCivilization->TakenTraits;
	}

	return TArray<FString>();
}
