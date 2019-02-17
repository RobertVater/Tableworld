// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Misc/TableHelper.h"
#include "TableGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUIShowLoadingScreen);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUIHideLoadingScreen);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FUIShowError, FText, Title, FText, Text);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FUIShowDialog, FText, Title, FText, Text, FText, ChoiceA, FText, ChoiceB);

UCLASS()
class TABLEWORLD_API UTableGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:

	virtual void Init() override;

	void LoadHumanNames();

	void PrepareNewGame(int32 nSeed, uint8 nWorldSize, bool nRiver, uint8 nRiverCount);
	void PrepareLoadGame(FString nSaveGame);

	void ClearLoadGame();
	void ClearWorldSettings();

	//UI Events
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowLoadingScreen();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideLoadingScreen();


	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowError(FText Title, FText Text);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowDialog(FText Title, FText Text, FText ChoiceA, FText ChoiceB);

	UFUNCTION(BlueprintCallable,BlueprintNativeEvent,Category = "Data")
	FTableBuilding getBuilding(const FName& ID, bool& bFound);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Data")
	FTableItem getItem(EItem Item, bool& bFound);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Data")
	FTableRescource getRescource(ETileRescources Rescource, bool& bFound);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Data")
	TArray<FTableBuilding> getBuildingsForAgeAndCategory(ETableAge Age, ETableTileCategory Category, bool& bFound);

	bool isLoadGame();
	FString getSaveGame();

	int32 getSeed();
	uint8 getWorldSize();
	bool hasRivers();
	uint8 getRiverCount();

	FName getHumanName(int32 Index);
	int32 getRandomHumanName();

	//Event
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "Events")
	FUIShowLoadingScreen Event_ShowLoadingScreen;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Events")
	FUIHideLoadingScreen Event_HideLoadingScreen;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Events")
	FUIShowError Event_ShowError;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Events")
	FUIShowDialog Event_ShowDialog;

protected:

	TArray<FName> Human_Names;

	bool bLoadGame = false;
	FString SaveGame;

	int32 Seed = 0;
	uint8 WorldSize = 8;
	bool bHasRiver = true;
	uint8 RiverCount = 3;
};
