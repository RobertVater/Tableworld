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

class UDialogChoice;
class UPopupInfo;

class UTableCivilization;
class UTableCivSavegame;

UCLASS()
class TABLEWORLD_API UTableGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Popup")
	TSubclassOf<UDialogChoice> DialogChoice_Class = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Popup")
	TSubclassOf<UPopupInfo> PopupInfo_Class = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Popup")
	TSubclassOf<UUserWidget> LoadingScreen_Class = nullptr;

	virtual void Init() override;
	void LoadCivilization(UTableCivSavegame* Save);

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
	UDialogChoice* CreateDialogChoice(FText Title, FText Text, FText A, FText B);

	UFUNCTION(BlueprintCallable, Category = "UI")
	UPopupInfo* CreatePopupInfo(FText Title, FText Text);

	UFUNCTION(BlueprintCallable, Category = "UI")
	UUserWidget* CreateLoadingScreen();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowError(FText Title, FText Text);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowDialog(FText Title, FText Text, FText ChoiceA, FText ChoiceB);

	UFUNCTION(BlueprintCallable,BlueprintNativeEvent,Category = "Data")
	FTableBuilding getBuilding(const FName& ID, bool& bFound);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Data")
	FTableItem getItem(EItem Item, bool& bFound);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Data")
	FCivTrait getTrait(const FString& ID, bool& bFound);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Data")
	FTableRescource getRescource(ETileRescources Rescource, bool& bFound);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Data")
	TArray<FTableBuilding> getBuildingsForAgeAndCategory(ETableAge Age, ETableTileCategory Category, bool& bFound);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	bool isLoadGame();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	FString getSaveGame();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	int32 getSeed();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	uint8 getWorldSize();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	bool hasRivers();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	uint8 getRiverCount();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	FName getHumanName(int32 Index, bool bIsMale);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	int32 getRandomHumanName(bool bIsMale);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	FText getCivName();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	FText getCivTitle();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	TArray<FString> getCivTraits();

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

	UPROPERTY()
	UTableCivilization* ActiveCivilization = nullptr;

	TArray<FName> HumanMale_Names;
	TArray<FName> HumanFemale_Names;

	bool bLoadGame = false;
	FString SaveGame;

	int32 Seed = 1337;
	uint8 WorldSize = 8;
	bool bHasRiver = true;
	uint8 RiverCount = 3;
};
