// Fill out your copyright notice in the Description page of Project Settings.

#include "TableWorldTable.h"
#include "Tableworld.h"
#include "Tile/TileData.h"
#include "DrawDebugHelpers.h"
#include "TableChunk.h"
#include "Misc/Math/FastNoise.h"
#include "Components/InstancedStaticMeshComponent.h"
#include <time.h>

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
	TileTypes.Add(ETileType::Rock);
	TileTypes.Add(ETileType::DirtRoad);

	SetupTilePixels(TileTypes);
	
	GenerateMap();
}

void ATableWorldTable::GenerateMap()
{
	InstancedRescourcesMesh.Empty();
	for(uint8 i = 0; i < (uint8)ETileRescources::Max; i++)
	{
		ETileRescources Res = (ETileRescources)i;
		if(Res != ETileRescources::None && Res != ETileRescources::Max)
		{
			UInstancedStaticMeshComponent* InstMesh = NewObject<UInstancedStaticMeshComponent>(this);
			if(InstMesh)
			{
				InstMesh->AttachToComponent(GetRootComponent(),FAttachmentTransformRules::KeepRelativeTransform);
				InstMesh->SetStaticMesh(RescourceMesh.FindRef(Res));
				InstMesh->RegisterComponent();

				InstancedRescourcesMesh.Add(Res, InstMesh);
			}
		}
	}
	
	//Setup the noise
	if(Noise)
	{
		Noise->SetSeed(FMath::RandRange(0, 9000));
		Noise->SetFrequency(0.05f);
	} 

	//Generate the base chunks
	GenerateChunks();

	int32 MaxXTiles = MaxSizeX * ChunkSize;
	int32 MaxYTiles = MaxSizeY * ChunkSize;

	//Generate the base tiles
	for(int32 x = 0; x < MaxXTiles; x++)
	{
		for(int32 y = 0; y < MaxYTiles; y++)
		{
			float PerlinValue = Noise->GetNoise2D(x, y);

			float WaterLevel = -0.7f;
			float RockLevel = 0.5f;
			
			//Place sand
			if(PerlinValue < WaterLevel+0.15f)
			{
				SetTile(x, y, ETileType::Sand);

				int32 dx = FMath::RandRange(-1, 1);
				int32 dy = FMath::RandRange(-1, 1);
				SetTileIfTile(x+dx, y+dy, ETileType::Sand, ETileType::Grass);
			}

			//Place water
			if(PerlinValue < WaterLevel)
			{
				SetTile(x, y, ETileType::Water);
			}

			//Rock
			if(PerlinValue > RockLevel)
			{
				SetTileIfTile(x, y, ETileType::Rock, ETileType::Grass);

				int32 dx = FMath::RandRange(-1, 1);
				int32 dy = FMath::RandRange(-1, 1);
				SetTileIfTile(x+dx, y+dy, ETileType::Rock, ETileType::Grass);
			}
		}
	}

	//Generate Rescources
	for (int32 x = 0; x < MaxXTiles; x++)
	{
		for (int32 y = 0; y < MaxYTiles; y++)
		{
			float PerlinValue = Noise->GetNoise2D(x, y);

			float TreeTile = 0.3f;
			float RockChance = 0.15f;

			//Place a tree
			if (PerlinValue > TreeTile)
			{
				SetRescource(x, y, ETileRescources::Tree, 100, ETileType::Grass);
			}

			//Place iron ore!
			if(FMath::RandRange(0.0f,1.0f) <= RockChance)
			{
				SetRescource(x, y, ETileRescources::IronOre, 50, ETileType::Rock);
			}
		}
	}

	//Generate the chunk visuals
	for(int32 i = 0; i < Chunks.Num(); i++)
	{
		ATableChunk* Chunk = Chunks[i];
		if(Chunk)
		{
			//Generate the chunk mesh
			Chunk->GenerateChunkMesh();

			//Generate the chunk texture
			Chunk->UpdateChunkTexture();
		}
	}
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

TArray<UTileData*> ATableWorldTable::getTilesInRadius(int32 X, int32 Y, int32 Radius)
{
	TArray<UTileData*> Tiles;

	for(int32 x = -Radius; x <= Radius; x++)
	{
		for(int32 y  = -Radius; y <= Radius; y++)
		{
			if((x * x) + (y * y) <= (Radius * Radius))
			{
				UTileData* Tile = getTile(X + x, Y + y);
				if(Tile)
				{
					Tiles.Add(Tile);
				}
			}
		}
	}

	return Tiles;
}

TArray<UTileData*> ATableWorldTable::getRescourcesInRadius(int32 X, int32 Y, int32 Radius, ETileRescources Rescource)
{
	TArray<UTileData*> Tiles;

	for (int32 x = -Radius; x <= Radius; x++)
	{
		for (int32 y = -Radius; y <= Radius; y++)
		{
			if ((x * x) + (y * y) <= (Radius * Radius))
			{
				UTileData* Tile = getTile(X + x, Y + y);
				if (Tile)
				{
					if (Tile->getTileRescources() == Rescource) 
					{
						Tile->DebugHighlightTile(1.0f);
						Tiles.Add(Tile);
					}
				}
			}
		}
	}

	return Tiles;
}

bool ATableWorldTable::HarvestRescource(UTileData* Tile, int32 Amount)
{
	if(Tile)
	{
		if(Tile->getTileRescources() != ETileRescources::None)
		{
			int32 NewAmount = Tile->getTileRescourceAmount() - Amount;
			
			if(NewAmount <= 0)
			{
				//Remove the Instance from the rescource
				UInstancedStaticMeshComponent* Mesh = InstancedRescourcesMesh.FindRef(ETileRescources::Tree);
				if(Mesh)
				{
					Mesh->UpdateInstanceTransform(Tile->getTileRescourceIndex(), FTransform(FRotator::ZeroRotator, FVector(0, 0, -500.0f), FVector::OneVector), true, true, true);
				}

				Tile->ClearRescource();
				return true;
			}else
			{
				Tile->UpdateRescource(NewAmount);
			}
		}
	}

	return false;
}

void ATableWorldTable::SetRescource(int32 X, int32 Y, ETileRescources Res, int32 Amount, ETileType NeededType)
{
	if (Res != ETileRescources::None)
	{
		if (Amount > 0)
		{
			UTileData* Tile = getTile(X, Y);
			if (Tile)
			{
				if (Tile->getTileType() == NeededType) 
				{
					if (!Tile->HasRescource()) 
					{
						UInstancedStaticMeshComponent* Mesh = InstancedRescourcesMesh.FindRef(Res);
						if (Mesh)
						{
							float OffsetX = 25.0f;
							float OffsetY = 25.0f;
							float RX = FMath::RandRange(-OffsetX, OffsetX);
							float RY = FMath::RandRange(-OffsetY, OffsetY);
							
							FTransform Trans;
							FRotator Rot = FRotator(FMath::RandRange(-2.5f,2.5f), FMath::RandRange(0.0f, 360.0f), 0.0f);
							float Scale = FMath::RandRange(0.9f, 1.1f);

							Trans.SetLocation(Tile->getWorldCenter() + FVector(RX,RY,0.0f));
							Trans.SetRotation(Rot.Quaternion());
							Trans.SetScale3D(FVector(Scale, Scale, Scale));

							int32 Index = Mesh->AddInstanceWorldSpace(Trans);

							Tile->SetRescource(Index, Res, Amount);
						}
					}
				}
			}
		}
	}
}

void ATableWorldTable::SetTile(int32 X, int32 Y, ETileType type, bool bUpdateTexture)
{
	ATableChunk* Chunk = getChunkForTile(X, Y);
	if (Chunk)
	{
		return Chunk->SetTile(X, Y, type, bUpdateTexture);
	}
}

void ATableWorldTable::SetTileIfTile(int32 X, int32 Y, ETileType NewTile, ETileType IfTile)
{
	ATableChunk* Chunk = getChunkForTile(X, Y);
	if (Chunk)
	{
		return Chunk->SetTileIfTile(X, Y, NewTile, IfTile, false);
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

	TArray<FColor> NullArray;

	for(int32 i = 0; i < (TilesInPixels*TilesInPixels) ; i++)
	{
		NullArray.Add(FColor::Red);
	}

	return NullArray;
}

UFastNoise* ATableWorldTable::getNoise()
{
	return Noise;
}

TArray<UTileData*> ATableWorldTable::FindPath(FVector2D StartTileCord, FVector2D EndTileCord, TArray<ETileType> ForbidenTiles, bool bAllowDiag, bool bIgnoreWeigths)
{
	DebugWarning("Starting Search!");
	clock_t t = clock();
	
	UTileData* StartTile = getTile((int32)StartTileCord.X, (int32)StartTileCord.Y);
	UTileData* EndTile = getTile((int32)EndTileCord.X, (int32)EndTileCord.Y);

	if(StartTile)
	{
		if (EndTile)
		{
			if(StartTile->IsBlocked() || EndTile->IsBlocked())
			{
				//Start or end is blocked
				return TArray<UTileData*>();
			}

			if(ForbidenTiles.Contains(StartTile->getTileType())  || ForbidenTiles.Contains(EndTile->getTileType()) )
			{
				//We tried to build a path on forbidden tiles!
				return TArray<UTileData*>();
			}
		}
	}

	TArray<UTileData*> ClosedSet;
	TArray<UTileData*> OpenSet;

	OpenSet.Add(StartTile);

	int32 MaxTries = 0;
	while(OpenSet.Num() > 0)
	{
		UTileData* CurrentTile = OpenSet[0];
		if(CurrentTile)
		{
			for(int32 i = 1; i < OpenSet.Num(); i++)
			{
				if(OpenSet[i]->getFCost() < CurrentTile->getFCost() || OpenSet[i]->getFCost() == CurrentTile->getFCost() && OpenSet[i]->getHCost() < CurrentTile->getHCost())
				{
					CurrentTile = OpenSet[i];
				}
			}

			OpenSet.Remove(CurrentTile);
			ClosedSet.Add(CurrentTile);

			if(CurrentTile == EndTile)
			{
				t = clock() - t;
				DebugWarning("Found! It took " + FString::SanitizeFloat((float)t / CLOCKS_PER_SEC ) + " secconds!");
				TArray<UTileData*> Path  = RetracePath(StartTile,EndTile);

				return Path;
			}

			//Check neighbours
			TArray<UTileData*> Neighbours = GetNeighbours(CurrentTile, bAllowDiag);
			for(int32 j = 0; j < Neighbours.Num(); j++)
			{
				UTileData* Neighbour = Neighbours[j];
				if(Neighbour)
				{
					if(Neighbour->IsBlocked() || ClosedSet.Contains(Neighbour) || ForbidenTiles.Contains(Neighbour->getTileType()))
					{
						continue;
					}

					int32 newMovementCost = CurrentTile->getGCost() + getDistance(CurrentTile, Neighbour);
					if(!bIgnoreWeigths)
					{
						newMovementCost += Neighbour->getMovementCost();
					}

					if(newMovementCost < Neighbour->getGCost() || !OpenSet.Contains(Neighbour))
					{
						Neighbour->GCost = newMovementCost;
						Neighbour->HCost = getDistance(Neighbour, EndTile);

						Neighbour->PathParent = CurrentTile;

						OpenSet.Add(Neighbour);
					}
				}
				
			}

			//MaxTries++;
			if(MaxTries >= 500)
			{
				DebugError("Path took too long to find. Abort!");
				return  TArray<UTileData*>();
			}
		}
	}

	return TArray<UTileData*>();
}

TArray<UTileData*> ATableWorldTable::RetracePath(UTileData* Start, UTileData* End)
{
	TArray<UTileData*> Path;
	UTileData* Current = End;

	if(Start && End)
	{
		while(Current != Start)
		{
			Path.Add(Current);
			Current = Current->PathParent;
		}
	}
	

	return Path;
}

TArray<UTileData*> ATableWorldTable::GetNeighbours(UTileData* Node, bool bAllowDiag)
{
	TArray<UTileData*> Neighbours;
	
	if (Node)
	{
		if (bAllowDiag)
		{
			for (int32 x = -1; x <= 1; x++)
			{
				for (int32 y = -1; y <= 1; y++)
				{
					if (x == 0 && y == 0)
					{
						continue;
					}

					int32 CheckX = Node->getX() + x;
					int32 CheckY = Node->getY() + y;

					Neighbours.Add(getTile(CheckX, CheckY));
				}
			}

			return Neighbours;
		}
		int32 X = Node->getX();
		int32 Y = Node->getY();

		Neighbours.Add(getTile(X - 1, Y));
		Neighbours.Add(getTile(X + 1, Y));
		Neighbours.Add(getTile(X, Y-1));
		Neighbours.Add(getTile(X, Y+1));
	}

	return Neighbours;
}

int32 ATableWorldTable::getDistance(UTileData* TileA, UTileData* TileB)
{
	if (TileA && TileB) 
	{
		int32 DistX = FMath::Abs(TileA->getX() - TileB->getX());
		int32 DistY = FMath::Abs(TileA->getY() - TileB->getY());

		if(DistX > DistY)
		{
			return 14 * DistY + 10 * (DistX - DistY);
		}else
		{
			return 14 * DistX + 10 * (DistY - DistX);
		}
	}

	return 0;
}
