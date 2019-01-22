// Fill out your copyright notice in the Description page of Project Settings.

#include "TableHelper.h"
#include "World/Tile/GrassTile.h"
#include "World/Tile/WaterTile.h"
#include "World/Tile/SandTile.h"
#include "World/Tile/RockTile.h"
#include "World/Tile/DirtRoadTile.h"

TSubclassOf<UTileData> UTableHelper::getTileClass(ETileType Type)
{
	switch(Type)
	{
	case ETileType::Grass: return UGrassTile::StaticClass();
	case ETileType::Water: return UWaterTile::StaticClass();
	case ETileType::Sand: return USandTile::StaticClass();
	case ETileType::Rock: return URockTile::StaticClass();
	case ETileType::DirtRoad: return UDirtRoadTile::StaticClass();
	}
	
	DebugError("Failed to create a class for tile " + FString::FromInt((uint8)Type) + ". So we used a basic tile instead!");
	return UTileData::StaticClass();
}

int32 UTableHelper::getTileDistance(UTileData* TileA, UTileData* TileB)
{
	if (TileA && TileB)
	{
		int32 DistX = FMath::Abs(TileA->getX() - TileB->getX());
		int32 DistY = FMath::Abs(TileA->getY() - TileB->getY());

		if (DistX > DistY)
		{
			return 14 * DistY + 10 * (DistX - DistY);
		}
		else
		{
			return 14 * DistX + 10 * (DistY - DistX);
		}
	}

	return 0;
}

int32 UTableHelper::getDistance(int32 X, int32 Y, int32 EX, int32 EY)
{
	return (FVector(X, Y, 0) - FVector(EX, EY, 0)).Size2D();
}
