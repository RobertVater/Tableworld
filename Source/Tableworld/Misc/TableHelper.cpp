// Fill out your copyright notice in the Description page of Project Settings.

#include "TableHelper.h"
#include "World/Tile/GrassTile.h"
#include "World/Tile/WaterTile.h"
#include "World/Tile/SandTile.h"
#include "World/Tile/RockTile.h"
#include "World/Tile/DirtRoadTile.h"
#include "World/Tile/DirtTile.h"
#include "World/TableWorldTable.h"

ATableWorldTable* UTableHelper::TableRef = nullptr;

bool UTableHelper::isDebug()
{
	return true;
}

void UTableHelper::Init(ATableWorldTable* nTableRef)
{
	TableRef = nTableRef;
}

TSubclassOf<UTileData> UTableHelper::getTileClass(ETileType Type)
{
	switch(Type)
	{
	case ETileType::Dirt: return UDirtTile::StaticClass();
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

TArray<FVector2D> UTableHelper::ConvertTileArrayTo2DVectorArray(TArray<UTileData*> Tiles)
{
	TArray<FVector2D> VectorArray;

	for(int32 i = 0; i < Tiles.Num(); i++)
	{
		UTileData* Tile = Tiles[i];
		if(Tile)
		{
			VectorArray.Add(FVector2D(Tile->getX(), Tile->getY()));
		}
	}

	return VectorArray;
}

TArray<UTileData*> UTableHelper::Convert2DVectorArrayToTileArray(TArray<FVector2D> Tiles)
{
	TArray<UTileData*> VectorArray;

	if (TableRef) 
	{
		for (int32 i = 0; i < Tiles.Num(); i++)
		{
			FVector2D Tile = Tiles[i];

			VectorArray.Add(TableRef->getTile((int32)Tile.X, (int32)Tile.Y));
		}
	}

	return VectorArray;
}
