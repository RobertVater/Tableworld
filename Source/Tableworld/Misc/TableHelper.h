// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/DataTable.h"
#include "TableHelper.generated.h"

class ABuildableTile;

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
struct FTableBuilding : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	FName ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	UTexture2D* Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	FText Name = FText::AsCultureInvariant("Name is missing!");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	FText Tooltip = FText::AsCultureInvariant("Tooltip is missing!");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	TSubclassOf<ABuildableTile> TileClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	ETableAge RequiredAge = ETableAge::StoneAge;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	ETableTileCategory TileCategory = ETableTileCategory::Management;
};

UCLASS()
class TABLEWORLD_API UTableHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
};
