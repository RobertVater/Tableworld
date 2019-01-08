// Fill out your copyright notice in the Description page of Project Settings.

#include "TableWorldTable.h"
#include "Tableworld.h"
#include "Tile/TileData.h"
#include "DrawDebugHelpers.h"
#include "TableChunk.h"
#include "Misc/Math/FastNoise.h"

ATableWorldTable::ATableWorldTable()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
}

void ATableWorldTable::BeginPlay()
{
	Super::BeginPlay();

	Noise = NewObject<UFastNoise>(this);

	//Preload the pixel data
	TArray<ETileType> TileTypes;
	TileTypes.Add(ETileType::Grass);
	TileTypes.Add(ETileType::Water);
	TileTypes.Add(ETileType::Dirt);
	TileTypes.Add(ETileType::Sand);

	SetupTilePixels(TileTypes);
	
	GenerateMap();
}

void ATableWorldTable::GenerateMap()
{
	if(Noise)
	{
		Noise->SetSeed(FMath::RandRange(0, 1000));
	}

	GenerateChunks();
}

void ATableWorldTable::GenerateChunks()
{
	for (int32 Y = 0; Y < MaxSizeY; Y++) 
	{
		for (int32 X = 0; X < MaxSizeX; X++) 
		{
			//Spawn a new chunk
			ATableChunk* Chunk = GetWorld()->SpawnActor<ATableChunk>(ATableChunk::StaticClass(), FVector((X * ChunkSize) * TileSize, (Y * ChunkSize) * TileSize, 0), FRotator::ZeroRotator);
			if(Chunk)
			{
				//Setup the chunk using basic data
				Chunk->SetupChunk(X, Y, this);

				//Store the chunk in a array
				Chunks.Add(Chunk);
			}
		}
	}
}

void ATableWorldTable::SetupTilePixels(TArray<ETileType> TileTypes)
{
	TilePixels.Empty();
	
	if(TileSheet)
	{	
		int32 SheetX = TileSheet->GetSizeX();
		int32 SheetY = TileSheet->GetSizeY();
		
		FTexture2DMipMap& Mip = TileSheet->PlatformData->Mips[0];
		const FColor* FormatedColorData = static_cast<const FColor*>(Mip.BulkData.LockReadOnly());

		for(int32 i = 0; i < TileTypes.Num(); i++)
		{
			ETileType tile = TileTypes[i];
			
			FVector2D TileOffset = TileMaterials.FindRef(tile);
			int32 OffsetX = (int32)TileOffset.X * TilesInPixels;
			int32 OffsetY = (int32)TileOffset.Y * TilesInPixels;

			FTilePixels NewTilePixels;
			NewTilePixels.TileType = tile;
			
			for(int32 y = 0; y < TilesInPixels; y++)
			{
				for(int32 x  = 0; x < TilesInPixels; x++)
				{
					int32 XX = OffsetX + x;
					int32 YY = OffsetY + y;
					
					FColor Pixel = FormatedColorData[YY * SheetX + XX];

					NewTilePixels.Pixels.Add(Pixel);

					//DrawDebugPoint(GetWorld(), FVector(x, y, 0), 5, Pixel, false, 999, 0);
				}
			}

			TilePixels.Add(NewTilePixels);
		}

		//Clear up the ram
		Mip.BulkData.Unlock();

		//Clear the RawData
		//delete[] FormatedColorData;
	}
}

ATableChunk* ATableWorldTable::getChunkForTile(int32 X, int32 Y)
{
	for (int32 i = 0; i < Chunks.Num(); i++)
	{
		ATableChunk* Chunk = Chunks[i];
		if (Chunk)
		{
			int32 MinX = Chunk->getX() * ChunkSize;
			int32 MinY = Chunk->getY() * ChunkSize;

			int32 MaxX = MinX + ChunkSize;
			int32 MaxY = MinY + ChunkSize;
			if (X >= MinX && X < MaxX)
			{
				if (Y >= MinY && Y < MaxY)
				{
					//The tile is in the border
					return Chunk;
				}
			}
		}
	}

	return nullptr;
}

UTileData* ATableWorldTable::getTile(int32 X, int32 Y)
{
	ATableChunk* Chunk = getChunkForTile(X, Y);
	if(Chunk)
	{
		return Chunk->getTile(X, Y);
	}

	return nullptr;
}

void ATableWorldTable::SetTile(int32 X, int32 Y, ETileType type)
{
	DebugWarning("Click tile: " + FString::FromInt(X) + "," + FString::FromInt(Y));
	
	ATableChunk* Chunk = getChunkForTile(X, Y);
	if (Chunk)
	{
		return Chunk->SetTile(X, Y, type);
	}
}

TArray<FColor> ATableWorldTable::getTilePixels(ETileType TileType)
{
	for(int32 i = 0; i < TilePixels.Num(); i++)
	{
		FTilePixels Tile = TilePixels[i];
		if(Tile.TileType == TileType)
		{
			return Tile.Pixels;
		}
	}

	return TArray<FColor>();
}

UFastNoise* ATableWorldTable::getNoise()
{
	return Noise;
}
