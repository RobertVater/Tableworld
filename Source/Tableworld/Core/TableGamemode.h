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

	void AddFloatingItem(EItem item, int32 Amount, FVector WorldLoc);

	void ModifyRescource(EItem Item, int32 AddAmount);

	UFUNCTION(BlueprintCallable, Category = "Game")
	virtual void SetCurrentAge(ETableAge mCurrentAge);

	UFUNCTION(BlueprintCallable, Category = "Building")
	virtual void SelectBuilding(FName SelectedBuildingID);

	UFUNCTION(BlueprintCallable, Category = "Tile")
	void SetTile(int32 X, int32 Y, ETileType type, bool bUpdateTexture = false);

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

	//Events

	UPROPERTY()
	FHudFloatingItem Event_FloatingItem;

	UPROPERTY()
	FStoredItemsUpdated Event_StoredItemsUpdated;

protected:

	FTableBuilding SelectedBuilding;

	UTableGameInstance* GI = nullptr;

	ATablePlayerController* PC = nullptr;
	ATablePlayerPawn* Player = nullptr;

	ATableWorldTable* Table = nullptr;

	ETableAge CurrentAge = ETableAge::StoneAge;
	int32 CurrentFunds = StartFunds;

	TMap<EItem, int32> StoredRescources;
};
