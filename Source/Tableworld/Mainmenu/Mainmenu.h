// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Mainmenu.generated.h"

class UTableCivilization;
class UDialogChoice;

UCLASS()
class TABLEWORLD_API AMainmenu : public AGameMode
{
	GENERATED_BODY()
	
public:

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Main")
	void SelectMenuState(int32 ActiveID);

	UFUNCTION(BlueprintCallable, Category = "Main")
	void ShowInfoBox(const FText& Title, const FText& Text);

	UFUNCTION()
	void CivNew_LeftPoints_Accept();

	UFUNCTION(BlueprintCallable, Category = "Main")
	UDialogChoice* ShowDialogBox(FText Title, FText Text, FText OptionA, FText OptionB);

	UFUNCTION(BlueprintCallable, Category = "Main")
	void NewGame();

	UFUNCTION(BlueprintCallable, Category = "Main")
	void LoadGame(FString SaveGame);

	UFUNCTION(BlueprintCallable, Category = "Main")
	bool DeleteGame(FString SaveGame);

	UFUNCTION(BlueprintCallable,Category = "Main")
	void LeaveMainmenu();

	UFUNCTION(BlueprintCallable, Category = "Main")
	TArray<FLoadedSavegame> ListSaveGames();

	UFUNCTION(BlueprintCallable, Category = "Main")
	TArray<FLoadedCiv> ListCivs();

	UFUNCTION(BlueprintCallable, Category = "Main")
	void SetSeed(int32 nUsedSeed);

	UFUNCTION(BlueprintCallable, Category = "Main")
	void SetWorldSize(uint8 nWorldSize);

	UFUNCTION(BlueprintCallable, Category = "Main")
	void SetRivers(bool bnGenRiver, uint8 nRiverCount);

	UFUNCTION(BlueprintCallable, Category = "Main")
	UTexture2D* GenerateMapPreview();

	UFUNCTION(BlueprintCallable, Category = "Civilization")
	void CreateCiv();

	UFUNCTION(BlueprintCallable, Category = "Civilization")
	bool SetCivDetails(FText Name, FText Title);

	UFUNCTION(BlueprintCallable, Category = "Civilization")
	void SaveNewCiv();

	UFUNCTION(BlueprintCallable, Category = "Civilization")
	bool DeleteCiv(FString SaveGame);

	UFUNCTION(BlueprintCallable, Category = "Civilization")
	void FinalizeCiv();

	UFUNCTION(BlueprintCallable, Category = "Main")
	bool BuyTrait(FCivTrait Trait);

	UFUNCTION(BlueprintCallable, Category = "Main")
	bool RefundTrait(FCivTrait Trait);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	int32 getCivPoints();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	TArray<FString> getCivTraits();


protected:

	UPROPERTY()
	UTableCivilization* Civilization = nullptr;

	int32 UsedSeed = 0;
	int32 WorldSize = 8;
	bool bGenerateRivers = true;
	uint8 RiverCount = 4;

	UPROPERTY()
	UTexture2D* MapPreview = nullptr;
};
