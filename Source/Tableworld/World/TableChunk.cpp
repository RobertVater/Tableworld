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

void ATableChunk::CleanupMemory()
{
	ChunkTexture->RemoveFromRoot();
	ChunkTexture = nullptr;
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
			AddPlane(x, y, i, Vertices, UVs);

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

	if (!ChunkTexture) 
	{
		ChunkTexture = UTexture2D::CreateTransient(TextureSize, TextureSize);
		ChunkTexture->AddToRoot();
		ChunkTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
		ChunkTexture->SRGB = false;
		ChunkTexture->Filter = TextureFilter::TF_Nearest;
		ChunkTexture->UpdateResource();
	}
}

void ATableChunk::AddPlane(float X, float Y, int32 i, TArray<FVector>& Verts, TArray<FVector2D>& UVs)
{
	//Generate a mesh section
	float VertX = (X * TileSize);
	float VertY = (Y * TileSize);

	int32 Next = i + ChunkSize + 1;
	Vertices[i] = FVector(VertX, VertY, 0.0f);

	if (Vertices.IsValidIndex(Next)) 
	{
		Vertices[Next] = FVector(VertX, VertY, 0.0f);
	}

	float UVX = (float)X / ChunkSize;
	float UVY = (float)Y / ChunkSize;

	UVs.Add(FVector2D(UVX, UVY));
}

void ATableChunk::AddCube(float X, float Y, float Heigth, int32 i, TArray<FVector>& Verts, TArray<FVector2D>& UVs)
{
	
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

			uint8* Pixels = new uint8[TextureSize * TextureSize * 4];
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

								int32 PixelIndex = (TileY + py) * TextureSize + (TileX + px);
								Pixels[4 * PixelIndex + 2] = PixelColor.R;
								Pixels[4 * PixelIndex + 1] = PixelColor.G;
								Pixels[4 * PixelIndex + 0] = PixelColor.B;
								Pixels[4 * PixelIndex + 3] = PixelColor.A;
							}
						}
					}
				}
			}

			FTexture2DMipMap& Mip = ChunkTexture->PlatformData->Mips[0];
			Mip.BulkData.Lock(LOCK_READ_WRITE);

			uint8* TextureData = (uint8*)Mip.BulkData.Realloc(TextureSize*TextureSize * 4);
			FMemory::Memcpy(TextureData, Pixels, sizeof(uint8) * (TextureSize * TextureSize * 4));
			Mip.BulkData.Unlock();
			ChunkTexture->UpdateResource();

			delete[] Pixels;

			DynamicMaterial->SetTextureParameterValue("Texture", ChunkTexture);

		}
	}
}

void ATableChunk::SetTile(int32 X, int32 Y, ETileType NewTileType, bool bUpdateMaterial, bool bModifyTile)
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

					if(bModifyTile)
					{
						NewTile->SetModified();
					}

					Tiles[Tile->getLocalY() * ChunkSize + Tile->getLocalX()] = NewTile;

					if (bUpdateMaterial)
					{
						//Update minimap
						if (ParentTable)
						{
							TArray<UTileData*> ModifiedTiles;
							ModifiedTiles.Add(NewTile);

							ParentTable->UpdateMinimap(ModifiedTiles);
						}
					}
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
	UTileData* Tile = getLocalTile(X, Y);
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

UTileData* ATableChunk::getTile(int32 WorldX, int32 WorldY)
{
	if (Tiles.Num() > 0)
	{
		int32 LocalX = WorldX - (getX() * ChunkSize);
		int32 LocalY = WorldY - (getY() * ChunkSize);

		int32 Index = LocalY * ChunkSize + LocalX;
		if (Tiles.IsValidIndex(Index))
		{
			return Tiles[Index];
		}
	}

	return nullptr;
}

UTileData* ATableChunk::getLocalTile(int32 X, int32 Y)
{
	int32 Index = X * ChunkSize + Y;
	if (Tiles.IsValidIndex(Index))
	{
		return Tiles[Index];
	}

	return nullptr;
}
