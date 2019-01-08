// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Tableworld.h"
#include "Misc/TableHelper.h"
#include "TileData.generated.h"

class ATableChunk;
class ABuildableTile;

UCLASS()
class TABLEWORLD_API UTileData : public UObject
{
	GENERATED_BODY()
	
public:

	void Set(int32 nX, int32 nY, int32 nArrayIndex, ETileType nTileType, ATableChunk* nTable);

	void UpdateTile(int32 nArrayIndex, ETileType nTileType);

	void AddBuildableTile(ABuildableTile* nTileObject);

	ETileType getTileType();
	int32 getArrayIndex();

	int32 getX();
	int32 getY();

	int32 getWorldX();
	int32 getWorldY();
	FVector getWorldCenter();

	bool HasTileObject();

protected:
	
	//The building that is sitting on this tile
	ABuildableTile* TileObject = nullptr;

	ATableChunk* ParentChunk = nullptr;
	ETileType TileType = ETileType::Grass;
	int32 ArrayIndex = 0;

	int32 X = 0;
	int32 Y = 0;

	int32 WorldX = 0;
	int32 WorldY = 0;
};
