// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"

struct FGeneratedMapTile
{
	int32 X = 0;
	int32 Y = 0;
	ETileType TileType = ETileType::Grass;
	ETileRescources Resscource = ETileRescources::None;
	uint8 ResscourceAmount = 0;
};

class TABLEWORLD_API MapGenerator
{
public:

	static uint8 TileSize;
	static uint8 ChunkSize;
	static uint8 RiverSize;

	//Base tile Noise level
	static float WaterLevel;
	static float SandLevel;
	static float RockLevel;

	//Rescource Noise Level
	static float TreeValue;
	static float CoppperValue;
	static float TinValue;
	static float BerrieValue;

	MapGenerator();

	static TArray<FGeneratedMapTile> GenerateMap(int32 Seed, int32 WorldSize, bool bHasRiver, uint8 RiverCount);

	static int32 getTileIndex(int32 X, int32 Y, int32 WorldSize);
	static FGeneratedMapTile CreateTile(int32 X, int32 Y, ETileType Type = ETileType::Grass, ETileRescources Res = ETileRescources::None, uint8 Amount = 0);
	static FGeneratedMapTile getTile(TArray<FGeneratedMapTile> Array, uint8 Size, int32 X, int32 Y);
	static void SetTile(TArray<FGeneratedMapTile>& Array, int32 Size, int32 X, int32 Y, ETileType Type = ETileType::Grass, ETileRescources Res = ETileRescources::None, uint8 Amount = 0, ETileType IfType = ETileType::Max);
	static void SetRescourceInClump(TArray<FGeneratedMapTile>& Array, int32 Size, int32 X, int32 Y, int32 Range, ETileRescources Res, uint8 Amount, ETileType IfTile = ETileType::Max);

	static FColor getTileColor(ETileType Type, ETileRescources Rescource = ETileRescources::None);
};
