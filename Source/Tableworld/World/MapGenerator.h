// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"

struct FGeneratedMapTile
{
	int32 X = 0;
	int32 Y = 0;
	ETileType TileType = ETileType::Grass;
	ETileRescources Resscource = ETileRescources::None;
	float NoiseValue = -1;
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
	static void SetTile(TArray<FGeneratedMapTile>& Array, int32 Size, int32 X, int32 Y, ETileType Type = ETileType::Grass, ETileRescources Res = ETileRescources::None, ETileType IfType = ETileType::Max, float NoiseValue = 0.0f);
	static void SetRescourceInClump(TArray<FGeneratedMapTile>& Array, int32 Size, int32 X, int32 Y, int32 Range, ETileRescources Res, ETileType IfTile = ETileType::Max);

	static bool getTileArea(TArray<FGeneratedMapTile>& Array, int32 Size, TMap<ETileType, int32> RequiredTiles, int32 SizeX, int32 SizeY, int32& OutX, int32& OutY);

	static FColor getTileColor(ETileType Type, ETileRescources Rescource = ETileRescources::None);
};
