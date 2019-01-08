// Fill out your copyright notice in the Description page of Project Settings.

#include "TableChunk.h"
#include "../Plugins/Runtime/ProceduralMeshComponent/Source/ProceduralMeshComponent/Public/ProceduralMeshComponent.h"
#include "../Plugins/Runtime/ProceduralMeshComponent/Source/ProceduralMeshComponent/Public/KismetProceduralMeshLibrary.h"
#include "DrawDebugHelpers.h"
#include "TableWorldTable.h"
#include "Tile/TileData.h"
#include "Misc/TableHelper.h"
#include "Misc/Math/FastNoise.h"

ATableChunk::ATableChunk()
{
	PrimaryActorTick.bCanEverTick = false;

	ChunkMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ChunkMesh"));
	RootComponent = ChunkMesh;
}

void ATableChunk::BeginPlay()
{
	Super::BeginPlay();
}

void ATableChunk::SetupChunk(uint8 nX, uint8 nY, ATableWorldTable* nParentTable)
{
	ParentTable = nParentTable;

	X = nX;
	Y = nY;

	ChunkSize = ParentTable->ChunkSize;

	GenerateChunk();
}

void ATableChunk::GenerateChunk()
{
	if (!ParentTable)return;

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FProcMeshTangent> Tangents;
	TArray<FVector2D> UVs;

	int32 ActualChunkSize = ChunkSize + 1;

	//Add the Verts
	Vertices.AddUninitialized((ActualChunkSize) * (ActualChunkSize));

	DebugWarning("---------------------------------");

	//Generate the chunk tiles
	int32 i = 0;
	for (int32 y = 0; y < ActualChunkSize; y++)
	{
		for (int32 x = 0; x < ActualChunkSize; x++)
		{
			//Generate a mesh section
			float VertX = (x * TileSize);
			float VertY = (y * TileSize);

			Vertices[i] = FVector(VertX, VertY, 0);

			float UVX = (float)x / ChunkSize;
			float UVY = (float)y / ChunkSize;

			UVs.Add(FVector2D(UVX, UVY));

			i++;
		}
	}

	for (int32 y = 0; y < ChunkSize; y++)
	{
		for (int32 x = 0; x < ChunkSize; x++)
		{
			//Create the tile data
			UTileData* Tile = NewObject<UTileData>(this, UTileData::StaticClass());
			if (Tile)
			{
				int32 TX = (getX() * ChunkSize) + x;
				int32 TY = (getY() * ChunkSize) + y;
				
				//Setup the tile data
				Tile->Set(TX, TY, Tiles.Num(), ETileType::Grass, this);
				Tiles.Add(Tile);
			}
		}
	}

	bool bGenerateRivers = ParentTable->bHasRiver;
	int32 RiverWidth = ParentTable->RiverSize;

	//Generate the Tris and Tangents / Normals of the plane
	UKismetProceduralMeshLibrary::CreateGridMeshTriangles(ActualChunkSize, ActualChunkSize, true, Triangles);
	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UVs, Normals, Tangents);

	//Create the mesh
	ChunkMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, TArray<FColor>(), Tangents, true);
	ChunkMesh->SetMaterial(0, ParentTable->ChunkMaterial);

	DynamicMaterial = ChunkMesh->CreateDynamicMaterialInstance(0);
	if(DynamicMaterial)
	{
		if (ParentTable) 
		{
			UpdateChunkTexture();
		}
	}
}

void ATableChunk::UpdateChunkTexture()
{
	uint8 TilesInPixels = ParentTable->TilesInPixels;

	//Create the texture
	int32 Resolution = 1;
	int32 TextureSize = ((ChunkSize * TilesInPixels));

	ChunkTexture = UTexture2D::CreateTransient(TextureSize, TextureSize);
	ChunkTexture->AddToRoot();
	ChunkTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	ChunkTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
	ChunkTexture->SRGB = false;
	ChunkTexture->Filter = TextureFilter::TF_Nearest;
	ChunkTexture->UpdateResource();

	FTexture2DMipMap& Mip = ChunkTexture->PlatformData->Mips[0];
	void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);

	TArray<FColor> Pixels;
	Pixels.AddUninitialized(TextureSize*TextureSize);
	for (int32 ty = 0; ty < ChunkSize; ty++)
	{
		for (int32 tx = 0; tx < ChunkSize; tx++)
		{
			UTileData* TileData = Tiles[tx + ty * ChunkSize];
			if (TileData)
			{
				ETileType TileType = TileData->getTileType();

				int32 TileX = (tx * TilesInPixels);
				int32 TileY = (ty * TilesInPixels);

				TArray<FColor> TilePixels = ParentTable->getTilePixels(TileType);

				for (int32 py = 0; py < TilesInPixels; py++)
				{
					for (int32 px = 0; px < TilesInPixels; px++)
					{
						Pixels[(TileY + py) * TextureSize + (TileX + px)] = TilePixels[py * TilesInPixels + px];
					}
				}
			}
		}
	}

	FMemory::Memcpy(Data, Pixels.GetData(), (TextureSize * TextureSize * 4));
	Mip.BulkData.Unlock();
	ChunkTexture->UpdateResource();

	DynamicMaterial->SetTextureParameterValue("Texture", ChunkTexture);
}

void ATableChunk::SetTile(int32 X, int32 Y, ETileType NewTileType)
{
	UTileData* Tile = getTile(X, Y);
	if(Tile)
	{
		if (Tile->getTileType() != NewTileType)
		{
			//Update the tile data
			Tile->UpdateTile(0, NewTileType);

			//Update the texture region of the tile to fit the new tile.
			UpdateChunkTexture();

			DrawDebugBox(GetWorld(), FVector((X * 100) + 50, (Y * 100) + 50, 0), FVector(50, 50, 50), FColor::Red, false, 1, 0, 5);
		}
	}
}

void ATableChunk::SetTileTexture(int32 X, int32 Y, ETileType Type)
{
	if (DynamicMaterial) 
	{
		if (ChunkTexture) 
		{
			if (ParentTable) 
			{
				UpdateChunkTexture();
			}
		}
	}
}

int32 ATableChunk::getX()
{
	return X;
}

int32 ATableChunk::getY()
{
	return Y;
}

UTileData* ATableChunk::getTile(int32 X, int32 Y)
{
	if(Tiles.Num() > 0)
	{
		for(int32 i = 0; i < Tiles.Num(); i++)
		{
			UTileData* Tile = Tiles[i];
			if(Tile)
			{
				if(Tile->getX() == X && Tile->getY() == Y)
				{
					return Tile;
				}
			}
		}
	}

	return nullptr;
}
