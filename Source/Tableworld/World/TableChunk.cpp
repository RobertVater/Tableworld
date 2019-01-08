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
			//DrawDebugString(GetWorld(), FVector(CX,CY, 0), FString::FromInt(i), NULL, FColor::Green, 99999, true);

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
				Tile->Set(TX, TY, x, y, ETileType::Grass, this);
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
}

void ATableChunk::UpdateChunkTexture()
{
	return;
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
								Pixels[(TileY + py) * TextureSize + (TileX + px)] = TilePixels[py * TilesInPixels + px];
							}
						}
					}
				}
			}

			FMemory::Memcpy(Data, Pixels.GetData(), (TextureSize * TextureSize * 4));
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
			Tile->UpdateTile(NewTileType);

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
		Tile->DebugHighlightTile(1.0f);
		
		int32 Size = ChunkSize+1;
		for(int32 x = 0; x < 2; x++)
		{
			for (int32 y = 0; y < 2; y++)
			{
				int32 VertX = (Tile->getLocalX() + x);
				int32 VertY = (Tile->getLocalY() + y);

				ChangeVertHeight(VertX, VertY, Heigth);

				DebugWarning(FString::FromInt(VertX) + "," + FString::FromInt(VertY));

				//X

				//+1 X
				if(VertX >= ChunkSize)
				{
					//get the tile +1 x from our tile.
					ATableChunk* PlusXChunk = ParentTable->getChunkForTile(X + 1, Y);
					if(PlusXChunk)
					{
						PlusXChunk->ChangeVertHeight(0, Tile->getLocalY(), Heigth);
						PlusXChunk->ChangeVertHeight(0, Tile->getLocalY() + 1, Heigth);

						PlusXChunk->UpdateMesh();
					}
				}

				//-1 X
				if (VertX <= 0)
				{
					//get the tile +1 x from our tile.
					ATableChunk* PlusXChunk = ParentTable->getChunkForTile(X - 1, Y);
					if (PlusXChunk)
					{
						PlusXChunk->ChangeVertHeight(Size-1, Tile->getLocalY(), Heigth);
						PlusXChunk->ChangeVertHeight(Size-1, Tile->getLocalY()+1, Heigth);

						PlusXChunk->UpdateMesh();
					}
				}

				//Y

				//+1 Y
				if (VertY >= ChunkSize)
				{
					//get the tile +1 x from our tile.
					ATableChunk* PlusXChunk = ParentTable->getChunkForTile(X, Y + 1);
					if (PlusXChunk)
					{
						PlusXChunk->ChangeVertHeight(Tile->getLocalX(), 0, Heigth);
						PlusXChunk->ChangeVertHeight(Tile->getLocalX() + 1, 0, Heigth);

						PlusXChunk->UpdateMesh();
					}
				}

				//-1 Y
				if (VertY <= 0)
				{
					//get the tile +1 x from our tile.
					ATableChunk* PlusXChunk = ParentTable->getChunkForTile(X, Y - 1);
					if (PlusXChunk)
					{
						PlusXChunk->ChangeVertHeight(Tile->getLocalX(), Size-1, Heigth);
						PlusXChunk->ChangeVertHeight(Tile->getLocalX() + 1, Size-1, Heigth);

						PlusXChunk->UpdateMesh();
					}
				}

				//Diag

				// +1 X | +1 Y
				if(VertX >= ChunkSize && VertY >= ChunkSize)
				{
					ATableChunk* PlusXChunk = ParentTable->getChunkForTile(X + 1, Y + 1);
					if (PlusXChunk)
					{
						PlusXChunk->ChangeVertHeight(0, 0, Heigth);

						PlusXChunk->UpdateMesh();
					}
				}

				// -1 X | +1 Y
				if (VertX <= 0 && VertY >= ChunkSize)
				{
					ATableChunk* PlusXChunk = ParentTable->getChunkForTile(X - 1, Y + 1);
					if (PlusXChunk)
					{
						PlusXChunk->ChangeVertHeight(Size-1, 0, Heigth);

						PlusXChunk->UpdateMesh();
					}
				}

				// +1 X | -1 Y
				if (VertX >= ChunkSize && VertY <= 0)
				{
					ATableChunk* PlusXChunk = ParentTable->getChunkForTile(X + 1, Y - 1);
					if (PlusXChunk)
					{
						PlusXChunk->ChangeVertHeight(0, Size -1, Heigth);

						PlusXChunk->UpdateMesh();
					}
				}

				// -1 X | -1 Y
				if (VertX <= 0 && VertY <= 0)
				{
					ATableChunk* PlusXChunk = ParentTable->getChunkForTile(X - 1, Y - 1);
					if (PlusXChunk)
					{
						PlusXChunk->ChangeVertHeight(Size - 1, Size - 1, Heigth);

						PlusXChunk->UpdateMesh();
					}
				}
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
		DrawDebugPoint(GetWorld(), VertLoc, 5, FColor::Red, false, 20, 0);

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
