// Fill out your copyright notice in the Description page of Project Settings.

#include "TileData.h"
#include "../TableWorldTable.h"
#include "Tableworld.h"
#include "Misc/TableHelper.h"
#include "../TableChunk.h"
#include "DrawDebugHelpers.h"

void UTileData::Set(int32 nX, int32 nY, int32 nLocalX, int32 nLocalY, ETileType nTileType, ATableChunk* nTable)
{
	X = nX;
	Y = nY;

	LocalX = nLocalX;
	LocalY = nLocalY;

	WorldX = X * 100;
	WorldY = Y * 100;

	TileType = nTileType;
	ParentChunk = nTable;
}

void UTileData::UpdateTile(ETileType nTileType)
{
	TileType = nTileType;
}

void UTileData::AddBuildableTile(ABuildableTile* nTileObject)
{
	TileObject = nTileObject;
}

void UTileData::DebugHighlightTile(float Time /*= 10.0f*/)
{
	DrawDebugBox(ParentChunk->GetWorld(), getWorldCenter(), FVector(50, 50, 50), FColor::Blue, false, Time, 0, 5.0f);
}

ETileType UTileData::getTileType()
{
	return TileType;
}

int32 UTileData::getX()
{
	return X;
}

int32 UTileData::getY()
{
	return Y;
}

int32 UTileData::getLocalX()
{
	return LocalX;
}

int32 UTileData::getLocalY()
{
	return LocalY;
}

int32 UTileData::getWorldX()
{
	return WorldX;
}

int32 UTileData::getWorldY()
{
	return WorldY;
}

FVector UTileData::getWorldCenter()
{
	return FVector(getWorldX() + 50, getWorldY() + 50, 0);
}

bool UTileData::HasTileObject()
{
	return (TileObject != nullptr);
}
