// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/DataTable.h"
#include "TableHelper.generated.h"

class ABuildableTile;
class UTileData;

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
	Max
};

UENUM(BlueprintType)
enum class ETileRescources : uint8
{
	None,
	Tree,
	IronOre,
	Fish,
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
struct FTilePixels
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TilePixels")
	ETileType TileType = ETileType::Grass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TilePixels")
	TArray<FColor> Pixels;
};

USTRUCT(BlueprintType)
struct FTableItem : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EItem ItemEnum = EItem::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText ItemName = FText::AsCultureInvariant("NAME IS MISSING");

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
	FText Name = FText::AsCultureInvariant("Name is missing!");

	//A short description of the building and what it does
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	FText Tooltip = FText::AsCultureInvariant("Tooltip is missing!");

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
};
