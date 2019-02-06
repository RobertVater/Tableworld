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

	void Set(int32 nX, int32 nY, int32 nLocalX, int32 nLocalY, ATableChunk* nTable);
	void SetHeigth(float nHeight);

	void CopyTileData(UTileData* CopyTile);
	void SetModified();

	void AddBuildableTile(ABuildableTile* nTileObject);
	void SetRescource(int32 Index, ETileRescources Type, int32 Amount);

	void GiveHarvester();
	void ClearHarvester();
	void UpdateRescource(int32 Amount);
	void ClearRescource();

	void DebugHighlightTile(float Time = 10.0f, FColor Color = FColor::Blue);

	int32 getTileRescourceIndex();
	ETileRescources getTileRescources();
	int32 getTileRescourceAmount();

	virtual ETileType getTileType();
	virtual float getBaseHeigth();
	virtual FColor getMinimapColor();
	float getHeigth();

	int32 getX();
	int32 getY();
	FVector2D getPositionAsVector();

	int32 getLocalX();
	int32 getLocalY();

	int32 getWorldX();
	int32 getWorldY();
	FVector getWorldCenter();
	ATableChunk* getParentChunk();

	int32 getFCost();
	int32 getHCost();
	int32 getGCost();
	virtual bool IsBlocked();
	virtual int32 getMovementCost();

	bool CanBuildOnTile();

	bool HasRescource();
	bool HasHarvester();
	bool HasTileObject();
	ABuildableTile* getTileObject();

	//True if this tile was modified at some point.
	bool isModified();
	bool HadRescource();
	int32 getLastRescourceIndex();

	//Pathfinding
	int32 GCost = 0;
	int32 HCost = 0;
	UTileData* PathParent = nullptr;

protected:

	//The building that is sitting on this tile
	ABuildableTile* TileObject = nullptr;

	ATableChunk* ParentChunk = nullptr;
	
	float Heigth = 0.0f;

	int32 X = 0;
	int32 Y = 0;

	uint8 LocalX = 0;
	uint8 LocalY = 0;

	int32 WorldX = 0;
	int32 WorldY = 0;

	bool bWasModified = false;

	bool bHasHarvester = false;
	int32 RescourceIndex = 0;
	ETileRescources TileRescource = ETileRescources::None;
	int32 RescourceCount = 0;

	ETileRescources LastResource = ETileRescources::None;
	int32 LastIndex = 0;
};
