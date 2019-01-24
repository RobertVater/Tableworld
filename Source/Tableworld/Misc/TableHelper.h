// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/DataTable.h"
#include "TableHelper.generated.h"

class ABuildableTile;
class UTileData;

UENUM(BlueprintType)
enum class ECreatureStatus: uint8
{
	Deactivated,
	Idle,
	Wandering,
	Working,
	ReturningGoods,
	Harvesting,
	GoingToWork
};

UENUM(BlueprintType)
enum class ETileType : uint8
{
	Grass,
	Water,
	Dirt,
	Sand,
	Rock,
	Woodland,
	Lava,
	Ash,
	DirtRoad,
	Max
};

UENUM(BlueprintType)
enum class EItem : uint8
{
	None,
	WoodLog,
	WoodPlank,
	Stone,
	StoneBricks,
	Fish,
	Charcoal,
	Clay,
	ClayBrick,
	IronOre,
	IronBar,
	CooperOre,
	CopperBar,
	BronzeBar,
	Steel,
	Coal,
	Thatch,
	Berries,
	Apples,
	DeerMeat,
	DeerCorpse,
	Water,
	Max
};

UENUM(BlueprintType)
enum class ETileRescources : uint8
{
	None,
	Tree,
	IronOre,
	Fish,
	Berries,
	Max
};

UENUM(BlueprintType)
enum class ETableAge : uint8
{
	StoneAge,
	DarkAge,
	MiddleAge,
	ImperialAge,
	IndustrialAge,
	ModernAge,
	NanoAge
};

UENUM(BlueprintType)
enum class ETableTileCategory : uint8
{
	Management,
	Harvester,
	Factory,
	Civil
};

UENUM(BlueprintType)
enum class ETableBuildingBuildType : uint8
{
	Actor,
	Tile
};

UENUM(BlueprintType)
enum class EBuildingInfoType : uint8
{
	Production,
	Storage,
	House
};

UENUM(BlueprintType)
enum class EToolbarTools : uint8
{
	None,
	Build,
	Demolish,
	Duplicate
};

USTRUCT(BlueprintType)
struct FNeededItems
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EItem NeededItem = EItem::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 NeededAmount = 0;
};

USTRUCT(BlueprintType)
struct FProductionItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	UTexture2D* Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	uint8 Amount = 1;
};

USTRUCT(BlueprintType)
struct FTilePixels
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TilePixels")
	ETileType TileType = ETileType::Grass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TilePixels")
	TArray<FColor> Pixels;
};

USTRUCT(BlueprintType)
struct FTableRescource : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rescource")
	ETileRescources RescourceEnum = ETileRescources::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rescource")
	FText RescourceName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rescource")
	UTexture2D* RescourceIcon = nullptr;
};

USTRUCT(BlueprintType)
struct FTableItem : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EItem ItemEnum = EItem::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	UTexture2D* ItemIcon;
};

USTRUCT(BlueprintType)
struct FTableBuilding : public FTableRowBase
{
	GENERATED_BODY()

	//The ID of the Building
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	FName ID;

	//The Icon that represents the Building
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	UTexture2D* Icon = nullptr;

	//The name of the building
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	FText Name;

	//A short description of the building and what it does
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	FText Tooltip;

	//The Actor class that spawns / represents the building.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	TSubclassOf<ABuildableTile> TileClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	TArray<FNeededItems> NeededItems;

	//True if we can drag build this building. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	bool bDragBuilding = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	bool bNeedsInfluence = true;

	//Actor "places" down all ghost actors and makes the building ready. Tile modifies the tile beneath the ghost actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	ETableBuildingBuildType BuildType = ETableBuildingBuildType::Actor;

	//The Tiletype that changes the tile under the ghost actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	ETileType BuildTileType = ETileType::DirtRoad;

	//how big in tiles this building is
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	FVector2D BuildingSize = FVector2D(1, 1);

	//A list of all tiles this building CANT be build!
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	TArray<ETileType> BlockedTiles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	ETableAge RequiredAge = ETableAge::StoneAge;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	ETableTileCategory TileCategory = ETableTileCategory::Management;
};

UCLASS()
class TABLEWORLD_API UTableHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	static TSubclassOf<UTileData> getTileClass(ETileType Type);
	static int32 getTileDistance(UTileData* TileA, UTileData* TileB);
	static int32 getDistance(int32 X, int32 Y, int32 EX, int32 EY);
};
