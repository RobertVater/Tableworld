// Fill out your copyright notice in the Description page of Project Settings.

#include "TableChunk.h"
#include "../Plugins/Runtime/ProceduralMeshComponent/Source/ProceduralMeshComponent/Public/ProceduralMeshComponent.h"
#include "../Plugins/Runtime/ProceduralMeshComponent/Source/ProceduralMeshComponent/Public/KismetProceduralMeshLibrary.h"
#include "DrawDebugHelpers.h"
#include "TableWorldTable.h"
#include "Tile/TileData.h"
#include "Misc/TableHelper.h"
#include "Misc/Math/FastNoise.h"
#include "Tile/GrassTile.h"

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

	GenerateTileData();
}

void ATableChunk::GenerateTileData()
{
	for (int32 y = 0; y < ChunkSize; y++)
	{
		for (int32 x = 0; x < ChunkSize; x++)
		{
			//Create the tile data
			UGrassTile* Tile = NewObject<UGrassTile>(this, UGrassTile::StaticClass());
			if (Tile)
			{
				int32 TX = (getX() * ChunkSize) + x;
				int32 TY = (getY() * ChunkSize) + y;

				//Setup the tile data
				Tile->Set(TX, TY, x, y, this);
				Tiles.Add(Tile);
			}
		}
	}
}

void ATableChunk::GenerateChunkMesh()
{
	if (!ParentTable)return;

	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FProcMeshTangent> Tangents;
	TArray<FVector2D> UVs;

	int32 ActualChunkSize = ChunkSize + 1;

	//Add the Verts
	Vertices.AddUninitialized((ActualChunkSize) * (ActualChunkSize));

	//Generate the chunk tiles
	int32 i = 0;
	for (int32 y = 0; y < ActualChunkSize; y++)
	{
		for (int32 x = 0; x < ActualChunkSize; x++)
		{
			//Generate a mesh section
			float VertX = (x * TileSize);
			float VertY = (y * TileSize);

			float CX = VertX + (X * (ChunkSize * TileSize));
			float CY = VertY + (Y * (ChunkSize * TileSize));

			Vertices[i] = FVector(VertX, VertY, 0.0f);

			float UVX = (float)x / ChunkSize;
			float UVY = (float)y / ChunkSize;

			UVs.Add(FVector2D(UVX, UVY));

			i++;
		}
	}

	//Generate the Tris and Tangents / Normals of the plane
	UKismetProceduralMeshLibrary::CreateGridMeshTriangles(ActualChunkSize, ActualChunkSize, true, Triangles);
	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UVs, Normals, Tangents);

	//Create the mesh
	ChunkMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, TArray<FColor>(), Tangents, true);
	ChunkMesh->SetMaterial(0, ParentTable->ChunkMaterial);

	DynamicMaterial = ChunkMesh->CreateDynamicMaterialInstance(0);

	uint8 TilesInPixels = ParentTable->TilesInPixels;

	//Create the texture
	int32 Resolution = 1;
	int32 TextureSize = ((ChunkSize * TilesInPixels));

	ChunkTexture = UTexture2D::CreateTransient(TextureSize, TextureSize);
	ChunkTexture->AddToRoot();
	ChunkTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	//ChunkTexture->MipMap = TextureMipGenSettings::TMGS_NoMipmaps;
	ChunkTexture->SRGB = false;
	ChunkTexture->Filter = TextureFilter::TF_Nearest;
	ChunkTexture->UpdateResource();
}

void ATableChunk::UpdateChunkTexture()
{
	if (ChunkTexture) 
	{
		if (DynamicMaterial)
		{
			uint8 TilesInPixels = ParentTable->TilesInPixels;

			//Create the texture
			int32 Resolution = 1;
			int32 TextureSize = ((ChunkSize * TilesInPixels));

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
								FColor PixelColor = TilePixels[py * TilesInPixels + px];
								PixelColor.R += FMath::RandRange(-0.5f, 0.5f);
								PixelColor.G += FMath::RandRange(-0.5f, 0.5f);
								PixelColor.B += FMath::RandRange(-0.5f, 0.5f);

								Pixels[(TileY + py) * TextureSize + (TileX + px)] = PixelColor;
							}
						}
					}
				}
			}

			if (Pixels.Num() > 0)
			{
				FMemory::Memcpy(Data, Pixels.GetData(), (TextureSize * TextureSize * 4));
			}
			
			Mip.BulkData.Unlock();
			ChunkTexture->UpdateResource();

			Pixels.Empty();
			delete[] Data;

			DynamicMaterial->SetTextureParameterValue("Texture", ChunkTexture);
		}
	}
}

void ATableChunk::SetTile(int32 X, int32 Y, ETileType NewTileType, bool bUpdateMaterial)
{
	UTileData* Tile = getTile(X, Y);
	if(Tile)
	{
		if (Tile->getTileType() != NewTileType)
		{
			//Update the tile data
			
			TSubclassOf<UTileData> TileClass = UTableHelper::getTileClass(NewTileType);
			if (TileClass) 
			{
				UTileData* NewTile = NewObject<UTileData>(this, TileClass);
				if (NewTile)
				{
					NewTile->CopyTileData(Tile);
					Tiles[Tile->getLocalY() * ChunkSize + Tile->getLocalX()] = NewTile;
				}
			}

			//Update the texture region of the tile to fit the new tile.
			if (bUpdateMaterial) 
			{
				UpdateChunkTexture();
			}
		}
	}
}


void ATableChunk::SetTileIfTile(int32 X, int32 Y, ETileType NewTileType, ETileType IfTile, bool bUpdateMaterial /*= true*/)
{
	UTileData* Tile = getTile(X, Y);
	if (Tile)
	{
		if (Tile->getTileType() == IfTile)
		{
			//Update the tile data
			UTileData* NewTile = NewObject<UTileData>(this, UTableHelper::getTileClass(NewTileType));
			if (NewTile)
			{
				NewTile->CopyTileData(Tile);
				Tiles[Tile->getLocalY() * ChunkSize + Tile->getLocalX()] = NewTile;
			}

			//Update the texture region of the tile to fit the new tile.
			if (bUpdateMaterial)
			{
				UpdateChunkTexture();
			}
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

void ATableChunk::ChangeTileHeight(int32 X, int32 Y, float Heigth)
{
	UTileData* Tile = getTile(X, Y);
	if(Tile)
	{
		//How many verts are in a chunk along x or y
		int32 Size = ChunkSize+1;

		for(int32 x = 0; x < 2; x++)
		{
			for (int32 y = 0; y < 2; y++)
			{
				int32 VertX = (Tile->getLocalX() + x);
				int32 VertY = (Tile->getLocalY() + y);

				ChangeVertHeight(VertX, VertY, Heigth);
				Tile->SetHeigth(Heigth);
			}
		}

		UpdateMesh();
	}
}

void ATableChunk::ChangeVertHeight(int32 X, int32 Y, float Heigth)
{
	int32 Size = ChunkSize + 1;
	int32 VertIndex = Y * Size + X;
	if (Vertices.IsValidIndex(VertIndex))
	{
		FVector VertLoc = Vertices[VertIndex];

		VertLoc.Z = Heigth;

		Vertices[VertIndex] = VertLoc;
	}
}

void ATableChunk::UpdateMesh()
{
	ChunkMesh->UpdateMeshSection(0, Vertices, TArray<FVector>(), TArray<FVector2D>(), TArray<FColor>(), TArray<FProcMeshTangent>());
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
