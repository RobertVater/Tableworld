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


protected:

	FTableBuilding SelectedBuilding;

	UTableGameInstance* GI = nullptr;

	ATablePlayerController* PC = nullptr;
	ATablePlayerPawn* Player = nullptr;

	ATableWorldTable* Table = nullptr;

	ETableAge CurrentAge = ETableAge::StoneAge;
	int32 CurrentFunds = StartFunds;
};
