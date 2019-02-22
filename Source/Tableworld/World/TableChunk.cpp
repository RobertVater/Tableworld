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

	ChunkSize = MapGenerator::ChunkSize;

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

	int32 Index = 0;
	Vertices.Empty();
	Triangles.Empty();
	Normals.Empty();
	Tangents.Empty();
	UVs.Empty();

	int32 ActualChunkSize = ChunkSize + 1;

	//Generate the chunk tiles
	if (ParentTable) 
	{
		for (int32 y = 0; y < ChunkSize; y++)
		{
			for (int32 x = 0; x < ChunkSize; x++)
			{
				int32 TX = (getX() * ChunkSize) + x;
				int32 TY = (getY() * ChunkSize) + y;

				int32 WX = (x * TileSize);
				int32 WY = (y * TileSize);
				
				UTileData* Tile = ParentTable->getTile(TX, TY);
				if (Tile)
				{
					//Our tile height
					int32 Height = Tile->getHeigth();

					AddCube(x, y, Height, Index);
				}
			}
		}
	}

	//Generate the Tris and Tangents / Normals of the plane
	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UVs, Normals, Tangents);

	//Create the mesh
	ChunkMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, TArray<FColor>(), Tangents, true);
	ChunkMesh->SetMaterial(0, ParentTable->ChunkMaterial);

	DynamicMaterial = ChunkMesh->CreateDynamicMaterialInstance(0);

	uint8 TilesInPixels = ParentTable->TilesInPixels;

	//Create the texture
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

void ATableChunk::AddCubeFace(int32 X, int32 Y, int32 Z, ETableDirections Dir, int32& i)
{
	int32 BlockHeigth = TileSize;

	float WX = X * TileSize;
	float WY = Y * TileSize;
	float WZ = Z * BlockHeigth;
	
	switch (Dir)
	{
	case ETableDirections::Top:
		Vertices.Add(FVector(WX, WY + TileSize, WZ));
		Vertices.Add(FVector(WX + TileSize, WY + TileSize, WZ));
		Vertices.Add(FVector(WX + TileSize, WY, WZ));
		Vertices.Add(FVector(WX, WY, WZ));
		break;

	case ETableDirections::North:
		Vertices.Add(FVector(WX + TileSize, WY + TileSize, WZ - BlockHeigth));
		Vertices.Add(FVector(WX + TileSize, WY + TileSize, WZ));
		Vertices.Add(FVector(WX, WY + TileSize, WZ));
		Vertices.Add(FVector(WX, WY + TileSize, WZ - BlockHeigth));
		break;

	case ETableDirections::East:
		Vertices.Add(FVector(WX + TileSize, WY, WZ - BlockHeigth));
		Vertices.Add(FVector(WX + TileSize, WY, WZ));
		Vertices.Add(FVector(WX + TileSize, WY + TileSize, WZ));
		Vertices.Add(FVector(WX + TileSize, WY + TileSize, WZ - BlockHeigth));
		break;

	case ETableDirections::South:
		Vertices.Add(FVector(WX, WY, WZ - TileSize));
		Vertices.Add(FVector(WX, WY, WZ));
		Vertices.Add(FVector(WX + TileSize, WY, WZ));
		Vertices.Add(FVector(WX + TileSize, WY, WZ - BlockHeigth));
		break;

	case ETableDirections::West:
		Vertices.Add(FVector(WX, WY + TileSize, WZ - BlockHeigth));
		Vertices.Add(FVector(WX, WY + TileSize, WZ));
		Vertices.Add(FVector(WX, WY, WZ));
		Vertices.Add(FVector(WX, WY, WZ - TileSize));
		break;

	case ETableDirections::Bottom:
		Vertices.Add(FVector(WX, WY, WZ - BlockHeigth));
		Vertices.Add(FVector(WX + TileSize, WY, WZ - BlockHeigth));
		Vertices.Add(FVector(WX + TileSize, WY + TileSize, WZ - BlockHeigth));
		Vertices.Add(FVector(WX, WY + TileSize, WZ - BlockHeigth));
		break;
	}

	AddTriangle(i * 4, i * 4 + 1, i * 4 + 2);
	AddTriangle(i * 4, i * 4 + 2, i * 4 + 3);

	float tUnit = 0.0625f;
	float CordX = (WX / TileSize) * tUnit;
	float CordY = (WY / TileSize) * tUnit;

	UVs.Add(FVector2D(CordX + tUnit, CordY));
	UVs.Add(FVector2D(CordX + tUnit, CordY + tUnit));
	UVs.Add(FVector2D(CordX, CordY + tUnit));
	UVs.Add(FVector2D(CordX, CordY));

	i++;
}

void ATableChunk::AddCube(int32 X, int32 Y, int32 Z, int32& i)
{
	int32 WX = X * TileSize;
	int32 WY = Y * TileSize;
	int32 WZ = Z * TileSize;

	int32 TX = (getX() * ChunkSize) + X;
	int32 TY = (getY() * ChunkSize) + Y;

	//Add the top plane
	AddCubeFace(X, Y, Z, ETableDirections::Top, i);
	int32 ZHeigth = Z;

	UTileData* North = ParentTable->getTile(TX, TY + 1);
	if (North)
	{
		if (Z > North->getHeigth())
		{
			while (ZHeigth > North->getHeigth()) 
			{
				AddCubeFace(X, Y, ZHeigth, ETableDirections::North, i);

				ZHeigth--;
			}
		}
	}

	UTileData* South = ParentTable->getTile(TX, TY - 1);
	if(South)
	{
		if(Z > South->getHeigth())
		{
			ZHeigth = Z;

			while (ZHeigth > South->getHeigth())
			{
				AddCubeFace(X, Y, ZHeigth, ETableDirections::South, i);

				ZHeigth--;
			}
		}
	}

	UTileData* East = ParentTable->getTile(TX + 1, TY);
	if (East)
	{
		if (Z > East->getHeigth())
		{
			ZHeigth = Z;
			
			while (ZHeigth > East->getHeigth()) 
			{
				AddCubeFace(X, Y, ZHeigth, ETableDirections::East, i);

				ZHeigth--;
			}
		}
	}

	UTileData* West = ParentTable->getTile(TX - 1, TY);
	if (West)
	{
		if (Z > West->getHeigth())
		{
			ZHeigth = Z;

			while (ZHeigth > West->getHeigth())
			{
				AddCubeFace(X, Y, ZHeigth, ETableDirections::West, i);

				ZHeigth--;
			}
		}
	}
}

void ATableChunk::AddTriangle(int32 a, int32 b, int32 c)
{
	Triangles.Add(a);
	Triangles.Add(b);
	Triangles.Add(c);
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

UTileData* ATableChunk::SetTile(int32 X, int32 Y, ETileType NewTileType, bool bUpdateMaterial, bool bModifyTile)
{
	UTileData* Tile = getTile(X, Y);
	if(Tile)
	{
		if (Tile->getTileType() != NewTileType)
		{
			//Update the tile data
			
			UTileData* ReturnTile = nullptr;
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
					
					ReturnTile = NewTile;
				}
			}

			//Update the texture region of the tile to fit the new tile.
			if (bUpdateMaterial) 
			{
				UpdateChunkTexture();
			}

			return ReturnTile;
		}
	}

	return nullptr;
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

void ATableChunk::UpdateTileHeights()
{
	return;

	for(int32 x = 0; x < ChunkSize; x++)
	{
		for (int32 y = 0; y < ChunkSize; y++)
		{
			UTileData* Tile = getLocalTile(x, y);
			if (Tile)
			{
				float Height = Tile->getBaseHeigth();
				ChangeTileHeight(x, y, Height);
			}
		}
	}

	RebuildMesh();
}

void ATableChunk::InsertPlane(int32 X, int32 Y, float Height)
{

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
	}
}

void ATableChunk::ChangeVertHeight(int32 Y, int32 X, float Heigth)
{
	int32 Size = ChunkSize + 1;

	//The next vert index
	int32 VertIndex = (Y * Size + X);
	FVector VertLoc = FVector(X * TileSize, Y * TileSize, Heigth);

	Vertices.Insert(VertLoc, VertIndex - 1);
	
	DrawDebugPoint(GetWorld(), VertLoc, 10, FColor::Red, false, 1);
}

void ATableChunk::RebuildMesh()
{
	//Clear the old mesh
	ChunkMesh->ClearAllMeshSections();

	int32 ActualChunkSize = ChunkSize + 1;

	UKismetProceduralMeshLibrary::CreateGridMeshTriangles(ActualChunkSize, ActualChunkSize, true, Triangles);
	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UVs, Normals, Tangents);
	ChunkMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, TArray<FVector2D>(), TArray<FVector2D>(), TArray<FVector2D>(), TArray<FColor>(), Tangents, true);
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