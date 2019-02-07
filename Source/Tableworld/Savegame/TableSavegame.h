// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "TableSavegame.generated.h"

USTRUCT(BlueprintType)
struct FTableSaveTile
{
	GENERATED_BODY()

	UPROPERTY()
	int32 X = 0;

	UPROPERTY()
	int32 Y = 0;

	UPROPERTY()
	ETileType Type = ETileType::Grass;

	UPROPERTY()
	bool bHasHarvester = false;

	UPROPERTY()
	int32 RescourceInstanceID = -1;

	UPROPERTY()
	ETileRescources RescourceType = ETileRescources::None;

	UPROPERTY()
	int32 RescourceHealth = 0;
};

USTRUCT(BlueprintType)
struct FTableSaveCreature
{
	GENERATED_BODY()

	UPROPERTY()
	FVector2D HomeTile = FVector2D(-1,-1);

	UPROPERTY()
	int32 PathIndex = 0;

	UPROPERTY()
	float MinDinstance = 0.0f;

	UPROPERTY()
	TArray<FVector> Path;

	UPROPERTY()
	TArray<FVector2D> LastCalcPath;

	UPROPERTY()
	FVector Location = FVector::ZeroVector;

	UPROPERTY()
	ECreatureStatus Status = ECreatureStatus::Idle;
};

USTRUCT(BlueprintType)
struct FTableSaveHarvesterCreature : public FTableSaveCreature
{
	GENERATED_BODY()

	UPROPERTY()
	float HarvestTimer = 0.0f;

	UPROPERTY()
	bool bHasCollected = false;

	UPROPERTY()
	bool bHasHarvestTile = false;

	UPROPERTY()
	int32 HarvestTileX = 0;

	UPROPERTY()
	int32 HarvestTileY = 0;
};

USTRUCT(BlueprintType)
struct FTableSaveHaulerCreature : public FTableSaveCreature
{
	GENERATED_BODY()

	UPROPERTY()
	FVector2D HaulTile = FVector2D(-1, -1);

	UPROPERTY()
	FName TargetBuildingUID = "";

	UPROPERTY()
	FName HomeBuildingUID = "";

	UPROPERTY()
	bool bHauledItems = false;

	UPROPERTY()
	TMap<EItem, int32> CarriedItems;
};

USTRUCT(BlueprintType)
struct FTableSaveBuilding
{
	GENERATED_BODY()

	UPROPERTY()
	FName UID = "";

	UPROPERTY()
	int32 TileX = 0;

	UPROPERTY()
	int32 TileY = 0;

	UPROPERTY()
	uint8 Rotation = 0;

	UPROPERTY()
	bool bRotated = false;

	UPROPERTY()
	FName BuildingID;
};

USTRUCT(BlueprintType)
struct FTableSaveInventoryBuilding : public FTableSaveBuilding
{
	GENERATED_BODY()

	UPROPERTY()
	int32 CurrentInventory = 0;
};

USTRUCT(BlueprintType)
struct FTableSaveHarvesterBuilding : public FTableSaveInventoryBuilding
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FTableSaveHarvesterCreature> Workers;
};

USTRUCT(BlueprintType)
struct FTableSaveProductionBuilding : public FTableSaveInventoryBuilding
{
	GENERATED_BODY()

	UPROPERTY()
	FName LastValidInventoryUID = "";

	UPROPERTY()
	TMap<EItem, int32> InputStorage;

	UPROPERTY()
	TMap<EItem, int32> OutputStorage;

	UPROPERTY()
	float ProductionTimer = 0.0f;

	UPROPERTY()
	float ResCheckTimer = 0.0f;

	UPROPERTY()
	TArray<FTableSaveHaulerCreature> Workers;
};

USTRUCT(BlueprintType)
struct FTableSaveStorageBuilding : public FTableSaveBuilding
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<EItem, int32> StoredItems;

	UPROPERTY()
	TArray<FReservedItem> ReservedItems;

	UPROPERTY()
	TArray<FTableSaveHaulerCreature> Workers;
};

USTRUCT(BlueprintType)
struct FTableSaveCityCenterBuilding : public FTableSaveStorageBuilding
{
	GENERATED_BODY()

	
};

UCLASS()
class TABLEWORLD_API UTableSavegame : public USaveGame
{
	GENERATED_BODY()
	
public:

	UPROPERTY()
	float PlayerX = 0;

	UPROPERTY()
	float PlayerY = 0;

	UPROPERTY()
	float ZoomAlpha = 0;

	UPROPERTY()
	int32 GameSpeed = 1;

	UPROPERTY()
	TMap<EItem, int32> GlobalItems;

	//The seed of the world
	UPROPERTY()
	int32 WorldSeed;

	//All the harvesters
	UPROPERTY()
	TArray<FTableSaveHarvesterBuilding> SavedHarvesters;

	//All the City Centers
	UPROPERTY()
	TArray<FTableSaveCityCenterBuilding> SavedCityCenters;

	//All of the Production Buildings
	UPROPERTY()
	TArray<FTableSaveProductionBuilding> SavedProduction;

	//All the saved rescources
	UPROPERTY()
	TArray<FTableSaveTile> SavedTiles;
};
