// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Misc/TableHelper.h"
#include "TableGamemode.generated.h"

class UTableGameInstance;
class ATableWorldTable;
class ATablePlayerController;
class ATablePlayerPawn;
class UTileData;
class ATableChunk;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FHudFloatingItem, EItem, Item, int32, Amount, FVector, WorldLocation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGameSpeedUpdated, int32, GameTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGameNotification, FTableNotification, NewNotification);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStoredItemsUpdated);

UCLASS()
class TABLEWORLD_API ATableGamemode : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	ETableAge StartAge = ETableAge::StoneAge;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	int32 StartFunds = 9000;

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void ModifyTime(int32 NewTime);

	UFUNCTION(BlueprintCallable)
	void StopTime();

	UFUNCTION(BlueprintCallable, Category = "SaveLoad")
	void SaveGame(FString SaveName);

	UFUNCTION(BlueprintCallable, Category = "SaveLoad")
	void LoadGame(FString SaveName);

	void AddNotification(FTableNotification NewNotifi);

	void AddFloatingItem(EItem item, int32 Amount, FVector WorldLoc);

	UFUNCTION(meta = (BlueprintThreadSafe))
	void ModifyRescource(EItem Item, int32 AddAmount);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	bool ConsumeRescource(TArray<FNeededItems> Rescources);

	UFUNCTION(BlueprintCallable, Category = "Game")
	virtual void SetCurrentAge(ETableAge mCurrentAge);

	UFUNCTION(BlueprintCallable, Category = "Building")
	virtual void SelectBuilding(FName SelectedBuildingID);

	UFUNCTION(BlueprintCallable, Category = "Tile")
	void SetTile(int32 X, int32 Y, ETileType type, bool bUpdateTexture = false, bool bModifyTile = false);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	ATablePlayerController* getPlayerController();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	ATablePlayerPawn* getPlayerPawn();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	UTableGameInstance* getGameInstance();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	ATableWorldTable* getTable();

	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "Getter")
	ETableAge getCurrentAge();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	ATableChunk* getChunkForTile(int32 X, int32 Y);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	UTileData* getTile(int32 X, int32 Y);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	FTableBuilding getSelectedBuilding();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	bool OwnNeededItems(TArray<FNeededItems> Items);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	TMap<EItem, int32> getStoredItems();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	int32 getStoredItemAmount(EItem Item, bool& bFound);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	int32 getGameTime();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	int32 getRandomSeed();

	//Events

	UPROPERTY()
	FHudFloatingItem Event_FloatingItem;

	UPROPERTY()
	FStoredItemsUpdated Event_StoredItemsUpdated;

	UPROPERTY()
	FGameSpeedUpdated Event_GameSpeedUpdated;

	UPROPERTY()
	FGameNotification Event_NewNotification;

protected:

	int32 WorldSeed;

	//The game speed before the game got paused
	int32 LastGameSpeed = 0;

	//The current speed level the game is running at
	int32 GameSpeed = 1;

	//True if the game is paused
	bool bGamePaused = false;

	FTableBuilding SelectedBuilding;

	UTableGameInstance* GI = nullptr;

	ATablePlayerController* PC = nullptr;
	ATablePlayerPawn* Player = nullptr;

	ATableWorldTable* Table = nullptr;

	ETableAge CurrentAge = ETableAge::StoneAge;
	int32 CurrentFunds = StartFunds;

	TMap<EItem, int32> StoredRescources;
};
