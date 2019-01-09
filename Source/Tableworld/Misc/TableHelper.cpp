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
