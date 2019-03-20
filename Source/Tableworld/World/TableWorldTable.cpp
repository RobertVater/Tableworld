// Fill out your copyright notice in the Description page of Project Settings.

#include "TableWorldTable.h"
#include "Tableworld.h"
#include "Tile/TileData.h"
#include "DrawDebugHelpers.h"
#include "TableChunk.h"
#include "Misc/Math/FastNoise.h"
#include "Components/InstancedStaticMeshComponent.h"
#include <time.h>
#include "Tile/Building/CityCentreBuilding.h"
#include "Tile/Building/BuildableTile.h"
#include "HeapSort.h"
#include "Component/HudManager.h"
#include "Player/TablePlayerController.h"
#include "UI/MainHud.h"
#include "Core/TableGamemode.h"
#include "Savegame/TableSavegame.h"
#include "Misc/TableHelper.h"
#include "MapGenerator.h"
#include "Core/TableGameInstance.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

ATableWorldTable::ATableWorldTable()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	WaterPlanes = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WaterPlanes"));
	WaterPlanes->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WaterPlanes->SetupAttachment(Root);
}

void ATableWorldTable::BeginPlay()
{
	Super::BeginPlay();

	UTableHelper::Init(this);
}

void ATableWorldTable::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(RescourceWobble.Num() > 0)
	{
		for(int32 i = 0; i < RescourceWobble.Num(); i++)
		{
			FRescourceWobble Wobble = RescourceWobble[i];

			if (Wobble.ParentChunk) 
			{
				UInstancedStaticMeshComponent* Mesh = Wobble.ParentChunk->getRescourceInstanceMesh(Wobble.Rescource);
				if (Mesh)
				{
					if (Wobble.Life <= 0)
					{
						//Reset instance
						FTransform Trans;
						Mesh->GetInstanceTransform(Wobble.InstanceIndex, Trans);

						Trans.SetRotation(FQuat(FRotator(0, Trans.GetRotation().Rotator().Yaw, 0)));

						Mesh->UpdateInstanceTransform(Wobble.InstanceIndex, Trans, false, true);

						RescourceWobble.RemoveAt(i);
						continue;
					}
					FTransform Trans;
					Mesh->GetInstanceTransform(Wobble.InstanceIndex, Trans, true);


					if (Wobble.Life >= Wobble.MaxLife)
					{
						float w = FMath::RandRange(-15.0f, 15.0f);
						Trans.SetRotation(FQuat(FRotator(w, Trans.GetRotation().Rotator().Yaw, w)));
					}


					float Pitch = Trans.GetRotation().Rotator().Pitch;
					float Roll = Trans.GetRotation().Rotator().Roll;
					float Yaw = Trans.GetRotation().Rotator().Yaw;

					float Speed = 20.0f;
					float PitchLerp = FMath::Lerp(Pitch, 0.0f, Speed * DeltaSeconds);
					float RollLerp = FMath::Lerp(Roll, 0.0f, Speed * DeltaSeconds);

					Trans.SetRotation(FQuat(FRotator(PitchLerp, Yaw, RollLerp)));

					Wobble.Life -= 1 * DeltaSeconds;

					Mesh->UpdateInstanceTransform(Wobble.InstanceIndex, Trans, true, true);
					RescourceWobble[i] = Wobble;
				}
			}
		}
	}
}

void ATableWorldTable::AddRescourceWobble(ETileRescources Rescource, int32 Index, float Life, ATableChunk* nParentChunk)
{
	FRescourceWobble NewWobble;
	NewWobble.Rescource = Rescource;
	NewWobble.InstanceIndex = Index;
	NewWobble.Life = Life;
	NewWobble.MaxLife = Life;
	NewWobble.ParentChunk = nParentChunk;

	RescourceWobble.Add(NewWobble);
}

void ATableWorldTable::InitTable(int32 nSeed, uint8 nWorldSize, bool bnHasRiver, uint8 nRiverCount)
{
	WaterPlanes->ClearInstances();
	
	Seed = nSeed;
	WorldSize = nWorldSize;
	bHasRiver = bnHasRiver;
	RiverCount = nRiverCount;

	//Reset all buildings
	for (int32 i = 0; i < Buildings.Num(); i++)
	{
		ABuildableTile* Building = Buildings[i];
		if (Building)
		{
			Building->Destroy();
		}
	}
	Buildings.Empty();

	//Reset all chunks
	for (int32 i = 0; i < Chunks.Num(); i++)
	{
		ATableChunk* Chunk = Chunks[i];
		if (Chunk)
		{
			Chunk->CleanupMemory();
			Chunk->Destroy();
		}
	}
	Chunks.Empty();

	//Clear all creatures
	TArray<AActor*> OutCreatures;
	UGameplayStatics::GetAllActorsOfClass(this, ABaseCreature::StaticClass(), OutCreatures);
	for(int32 i = 0; i < OutCreatures.Num(); i++)
	{
		AActor* Actor = OutCreatures[i];
		if(Actor)
		{
			Actor->Destroy();
		}
	}

	//Clear tile pixels
	TilePixels.Empty();

	//Clear all wobble
	RescourceWobble.Empty();

	//Create the noise and asign it a seed
	Noise = NewObject<UFastNoise>(this);
	Noise->SetSeed(getRandomSeed());

	//Preload the pixel data
	TArray<ETileType> TileTypes;
	TileTypes.Add(ETileType::Grass);
	TileTypes.Add(ETileType::Water);
	TileTypes.Add(ETileType::Dirt);
	TileTypes.Add(ETileType::Sand);
	TileTypes.Add(ETileType::Rock);
	TileTypes.Add(ETileType::DirtRoad);

	//Setup tile pixels for the tiletypes
	SetupTilePixels(TileTypes);

	//Generate the world
	GenerateMap();

	//Start the pathfinder
	Pathfinder = new Thread_Pathfinder(this);
}

void ATableWorldTable::GenerateMap()
{
	GenerateChunks();
	
	TArray<FGeneratedMapTile> Tiles = MapGenerator::GenerateMap(Seed, WorldSize, bHasRiver, RiverCount);

	for(int32 i = 0; i < Tiles.Num(); i++)
	{
		FGeneratedMapTile Data = Tiles[i];
		LoadTile(Data);
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

			Chunk->GenerateInstancedMesh(GrassSettings, RescourceMesh, GrassMesh);
		}
	}


	//Generate Minimap
	GenerateMinimap();
}

void ATableWorldTable::GenerateChunks()
{
	for (int32 Y = 0; Y < (int32)WorldSize; Y++) 
	{
		for (int32 X = 0; X < (int32)WorldSize; X++) 
		{
			//Spawn a new chunk
			ATableChunk* Chunk = GetWorld()->SpawnActor<ATableChunk>(ATableChunk::StaticClass(), FVector((X * MapGenerator::ChunkSize) * MapGenerator::TileSize, (Y * MapGenerator::ChunkSize) * MapGenerator::TileSize, 0), FRotator::ZeroRotator);
			if(Chunk)
			{
				//Setup the chunk using basic data
				Chunk->SetupChunk(X, Y, this);

				//Store the chunk in a array
				Chunks.Add(Chunk);

				//Add water
				FVector Location;

				Location.X = (((X * MapGenerator::ChunkSize) * MapGenerator::TileSize) + ((MapGenerator::ChunkSize / 2) * MapGenerator::TileSize));
				Location.Y = (((Y * MapGenerator::ChunkSize ) * MapGenerator::TileSize) + ((MapGenerator::ChunkSize / 2) * MapGenerator::TileSize));
				Location.Z = -15.0f;

				WaterPlanes->AddInstanceWorldSpace(FTransform(FRotator::ZeroRotator, Location));
			}
		}
	}
}

void ATableWorldTable::ClearMinimap()
{
	if(MinimapTexture)
	{
		MinimapTexture->RemoveFromRoot();
		MinimapTexture = nullptr;
	}
}

UTexture2D* ATableWorldTable::GenerateMinimap()
{
	int32 TextureSize = MapGenerator::ChunkSize * WorldSize;

	if (!MinimapTexture)
	{
		MinimapTexture = UTexture2D::CreateTransient(TextureSize, TextureSize);
		MinimapTexture->AddToRoot();
		MinimapTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
		MinimapTexture->SRGB = false;
		MinimapTexture->Filter = TextureFilter::TF_Nearest;
		MinimapTexture->UpdateResource();
	}

	//Go through each tile
	uint8* Pixels = new uint8[TextureSize*TextureSize * 4];

	for (int32 tx = 0; tx < TextureSize; tx++)
	{
		for (int32 ty = 0; ty < TextureSize; ty++)
		{
			UTileData* Tile = getTile(tx, ty);
			if (Tile)
			{
				FColor Color = MapGenerator::getTileColor(Tile->getTileType(), Tile->getTileRescources());

				int32 PixelIndex = ty * TextureSize + tx;
				Pixels[4 * PixelIndex + 2] = Color.R;
				Pixels[4 * PixelIndex + 1] = Color.G;
				Pixels[4 * PixelIndex + 0] = Color.B;
				Pixels[4 * PixelIndex + 3] = Color.A;
			}
		}
	}

	FTexture2DMipMap& Mip = MinimapTexture->PlatformData->Mips[0];
	Mip.BulkData.Lock(LOCK_READ_WRITE);

	uint8* TextureData = (uint8*)Mip.BulkData.Realloc(TextureSize*TextureSize * 4);
	FMemory::Memcpy(TextureData, Pixels, sizeof(uint8) * (TextureSize * TextureSize * 4));
	Mip.BulkData.Unlock();
	MinimapTexture->UpdateResource();

	delete[] Pixels;

	return MinimapTexture;
}

void ATableWorldTable::UpdateMinimap(TArray<UTileData*> ModifiedTiles)
{
	if(!MinimapTexture)
	{
		DebugError("Minimap Texture was null! Generated a new one");
		GenerateMinimap();
	}

	int32 TextureSize = MapGenerator::ChunkSize * WorldSize;

	FTexture2DMipMap& Mip = MinimapTexture->PlatformData->Mips[0];
	FColor* FormatedColorData = static_cast<FColor*>(Mip.BulkData.Lock(LOCK_READ_WRITE));

	for(int32 i = 0; i < ModifiedTiles.Num(); i++)
	{
		UTileData* Tile = ModifiedTiles[i];
		if(Tile)
		{
			int32 X = Tile->getX();
			int32 Y = Tile->getY();

			FColor Color = MapGenerator::getTileColor(Tile->getTileType(), Tile->getTileRescources());
			if(Tile->HasTileObject())
			{
				Color = Tile->getTileObject()->getMinimapColor();
			}

			int32 PixelIndex = Y * (TextureSize) + X;
			FormatedColorData[PixelIndex] = Color;
		}
	}

	//Update the minimap data

	uint8* TextureData = (uint8*)Mip.BulkData.Realloc(TextureSize*TextureSize * 4);
	FMemory::Memcpy(TextureData, FormatedColorData, sizeof(uint8) * (TextureSize * TextureSize * 4));
	Mip.BulkData.Unlock();
	MinimapTexture->UpdateResource();

	if (getPlayerController()) 
	{
		getPlayerController()->UpdateMinimap();
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
				}
			}

			TilePixels.Add(NewTilePixels);
		}

		//Clear up the ram
		Mip.BulkData.Unlock();
	}
}

void ATableWorldTable::SetMultipleTiles(TArray<UTileData*> Tiles)
{
	ATableChunk* LastChunk = nullptr;
	for (int32 i = 0; i < Tiles.Num(); i++)
	{
		UTileData* Tile = Tiles[i];
		if (Tile)
		{
			ATableChunk* Chunk = getChunkForTile(Tile->getX(), Tile->getY());
			if (Chunk)
			{
				if (!LastChunk)
				{
					LastChunk = Chunk;
					LastChunk->UpdateChunkTexture();
					continue;
				}

				if (Chunk != LastChunk)
				{
					LastChunk = Chunk;
					LastChunk->UpdateChunkTexture();
				}
			}
		}
	}

	//Refresh the minimap
	UpdateMinimap(Tiles);
}

ATableChunk* ATableWorldTable::getChunkForTile(int32 X, int32 Y)
{
	for (int32 i = 0; i < Chunks.Num(); i++)
	{
		ATableChunk* Chunk = Chunks[i];
		if (Chunk)
		{
			int32 MinX = Chunk->getX() * MapGenerator::ChunkSize;
			int32 MinY = Chunk->getY() * MapGenerator::ChunkSize;

			int32 MaxX = MinX + MapGenerator::ChunkSize;
			int32 MaxY = MinY + MapGenerator::ChunkSize;
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

TArray<UTileData*> ATableWorldTable::getTilesInRadius(int32 X, int32 Y, int32 Radius, ETileType TileType, bool bBlockRescources)
{
	TArray<UTileData*> Tiles;

	for (int32 x = -Radius; x <= Radius; x++)
	{
		for (int32 y = -Radius; y <= Radius; y++)
		{
			if ((x * x) + (y * y) < (Radius * Radius))
			{
				int32 XX = (X + x);
				int32 YY = (Y + y);

				UTileData* Tile = getTile(XX, YY);
				if (Tile)
				{
					if (Tile->getTileType() == TileType)
					{
						if (bBlockRescources)
						{
							if (Tile->HasRescource())
							{
								continue;
							}
						}

						Tiles.Add(Tile);
					}
				}
			}
		}
	}

	return Tiles;
}

TArray<UTileData*> ATableWorldTable::getRescourcesInRadius(int32 X, int32 Y, int32 Radius, TArray<ETileRescources> Rescource)
{
	TArray<UTileData*> Tiles;

	for (int32 x = -Radius; x <= Radius; x++)
	{
		for (int32 y = -Radius; y <= Radius; y++)
		{
			if ((x * x) + (y * y) < (Radius * Radius))
			{
				int32 XX = (X + x);
				int32 YY = (Y + y);

				UTileData* Tile = getTile(XX, YY);
				if (Tile)
				{
					if (Rescource.Contains(Tile->getTileRescources())) 
					{
						Tiles.Add(Tile);
					}
				}
			}
		}
	}

	return Tiles;
}

int32 ATableWorldTable::getNumTilesAroundTile(int32 X, int32 Y, ETileType Type)
{
	int32 Num = 0;
	for (int32 x = -1; x <= 1; x++)
	{
		for (int32 y = -1; y <= 1; y++)
		{
			UTileData* Tile = getTile(X + x, Y + y);
			if(Tile)
			{
				if(Tile->getTileType() == Type)
				{
					if(X+x != X && Y+y != Y)
					{
						Num++;
					}
				}
			}
		}
	}

	return Num;
}

bool ATableWorldTable::InInfluenceRange(int32 CenterX, int32 CenterY, int32 Radius, int32 X, int32 Y, FVector2D Size)
{
	int32 SizeX = (int32)Size.X / 2;
	int32 SizeY = (int32)Size.Y / 2;

	int32 TopLeftX = X - SizeX;
	int32 TopLeftY = Y - SizeY;
	int32 Dist = UTableHelper::getDistance(CenterX, CenterY, X, Y);

	for (int32 x = -Radius-1; x <= Radius; x++)
	{
		for (int32 y = -Radius-1; y <= Radius; y++)
		{
			if ((x * x) + (y * y) <= (Radius * Radius))
			{
				for (int32 bx = 0; bx < Size.X; bx++)
				{
					for (int32 by = 0; by < Size.Y; by++)
					{
						int32 XX = TopLeftX + bx;
						int32 YY = TopLeftY + by;
						int32 Distance = UTableHelper::getDistance(CenterX, CenterY, XX, YY);
						if (Distance >= Radius)
						{
							return false;
						}
					}
				}
			}
		}
	}

	return true;
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
				if (Tile->getParentChunk()) 
				{
					UInstancedStaticMeshComponent* Mesh = Tile->getParentChunk()->getRescourceInstanceMesh(Tile->getTileRescources());
					if (Mesh)
					{
						Mesh->UpdateInstanceTransform(Tile->getTileRescourceIndex(), FTransform(FRotator::ZeroRotator, FVector(0, 0, -500.0f), FVector::OneVector), true, true, true);
					}
				}

				Tile->ClearRescource();

				TArray<UTileData*> ModifiedTile;
				ModifiedTile.Add(Tile);

				UpdateMinimap(ModifiedTile);


				return true;
			}else
			{
				Tile->UpdateRescource(NewAmount);
			}
		}
	}

	return false;
}

void ATableWorldTable::SetRescource(int32 X, int32 Y, ETileRescources Res, ETileType NeededType)
{
	if (Res != ETileRescources::None)
	{
		if (getGameInstance())
		{
			UTileData* Tile = getTile(X, Y);
			if (Tile)
			{
				if (Tile->getTileType() == NeededType)
				{
					if (!Tile->HasRescource())
					{
						bool bFound = false;
						FTableRescource ResData = getGameInstance()->getRescource(Res, bFound);

						if (bFound)
						{
							Tile->SetRescource(Res, ResData.RescourceAmount, ResData.bUnlimitedRescources);
						}
					}
				}
			}
		}
	}
}

UTileData* ATableWorldTable::SetTile(int32 X, int32 Y, ETileType type, bool bUpdateTexture, bool bModifyTile)
{
	ATableChunk* Chunk = getChunkForTile(X, Y);
	if (Chunk)
	{
		return Chunk->SetTile(X, Y, type, bUpdateTexture, bModifyTile);
	}

	return nullptr;
}

void ATableWorldTable::SetTileIfTile(int32 X, int32 Y, ETileType NewTile, ETileType IfTile)
{
	ATableChunk* Chunk = getChunkForTile(X, Y);
	if (Chunk)
	{
		return Chunk->SetTileIfTile(X, Y, NewTile, IfTile, false);
	}
}

void ATableWorldTable::LoadTile(FGeneratedMapTile Data)
{
	UTileData* Tile = SetTile(Data.X, Data.Y, Data.TileType, false, false);

	if(Tile)
	{
		if(Tile->getTileType() == ETileType::Rock)
		{
			Tile->SetHeigth(getNumTilesAroundTile(Data.X, Data.Y, Tile->getTileType()) + 2);
		}
	}

	if (Data.Resscource != ETileRescources::None && Data.Resscource != ETileRescources::Max)
	{
		SetRescource(Data.X, Data.Y, Data.Resscource, Data.TileType);
	}
}

void ATableWorldTable::AddBuilding(ABuildableTile* nBuilding)
{
	Buildings.Add(nBuilding);
}

void ATableWorldTable::RemoveBuilding(ABuildableTile* nBuilding)
{
	Buildings.Remove(nBuilding);
}

void ATableWorldTable::ShowInfluenceGrid()
{
	TArray<UInfluenceComponent*> Influence = getInfluenceComponents();
	for(int32 i = 0; i < Influence.Num(); i++)
	{
		UInfluenceComponent* Inf = Influence[i];
		if(Inf)
		{
			Inf->ShowInfluenceRadius();
		}
	}
}

void ATableWorldTable::HideInfluenceGrid()
{
	TArray<UInfluenceComponent*> Influence = getInfluenceComponents();
	for (int32 i = 0; i < Influence.Num(); i++)
	{
		UInfluenceComponent* Inf = Influence[i];
		if (Inf)
		{
			Inf->HideInfluenceRadius();
		}
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

ATablePlayerController* ATableWorldTable::getPlayerController()
{
	if(!PC)
	{
		PC = Cast<ATablePlayerController>(GetWorld()->GetFirstPlayerController());
	}

	return PC;
}

ATableGamemode* ATableWorldTable::getGamemode()
{
	if(!GM)
	{
		GM = Cast<ATableGamemode>(UGameplayStatics::GetGameMode(this));
	}

	return GM;
}

UTableGameInstance* ATableWorldTable::getGameInstance()
{
	if(!GI)
	{
		GI = Cast<UTableGameInstance>(GetGameInstance());
	}

	return GI;
}

UFastNoise* ATableWorldTable::getNoise()
{
	return Noise;
}

TArray<UTileData*> ATableWorldTable::FindPath(FTablePathfindingRequest Request)
{
	clock_t t = clock();

	UTileData* StartTile = getTile((int32)Request.StartTile.X, (int32)Request.StartTile.Y);
	UTileData* EndTile = getTile((int32)Request.EndTile.X, (int32)Request.EndTile.Y);

	if (StartTile)
	{
		if (EndTile)
		{
			if (StartTile->IsBlocked(Request.bIgnoreRescources) || EndTile->IsBlocked(Request.bIgnoreRescources))
			{
				//Start or end is blocked
				return TArray<UTileData*>();
			}

			if (Request.ForbidenTiles.Contains(StartTile->getTileType()) || Request.ForbidenTiles.Contains(EndTile->getTileType()))
			{
				//We tried to build a path on forbidden tiles!
				return TArray<UTileData*>();
			}

			if (Request.AllowedTiles.Num() > 0)
			{
				if (!Request.AllowedTiles.Contains(StartTile->getTileType()) || !Request.AllowedTiles.Contains(EndTile->getTileType()))
				{
					//The start or end tile are not valid
					return TArray<UTileData*>();
				}
			}
		}
	}

	TArray<UTileData*> ClosedSet;
	TArray<UTileData*> OpenSet;

	OpenSet.Add(StartTile);

	int32 MaxTries = 0;
	while (OpenSet.Num() > 0)
	{
		UTileData* CurrentTile = OpenSet[0];
		if (CurrentTile)
		{
			for (int32 i = 1; i < OpenSet.Num(); i++)
			{
				if (OpenSet[i]->getFCost() < CurrentTile->getFCost() || OpenSet[i]->getFCost() == CurrentTile->getFCost() && OpenSet[i]->getHCost() < CurrentTile->getHCost())
				{
					CurrentTile = OpenSet[i];
				}
			}

			OpenSet.Remove(CurrentTile);
			ClosedSet.Add(CurrentTile);

			if (CurrentTile == EndTile)
			{
				t = clock() - t;
				DebugWarning("Found! It took " + FString::SanitizeFloat((float)t / CLOCKS_PER_SEC) + " secconds!");
				TArray<UTileData*> Path = RetracePath(StartTile, EndTile);

				Algo::Reverse(Path);
				return Path;
			}

			//Check neighbours
			TArray<UTileData*> Neighbours = GetNeighbours(CurrentTile, Request.bAllowDiag);
			for (int32 j = 0; j < Neighbours.Num(); j++)
			{
				UTileData* Neighbour = Neighbours[j];
				if (Neighbour)
				{
					if (Neighbour->IsBlocked(Request.bIgnoreRescources) || ClosedSet.Contains(Neighbour) || Request.ForbidenTiles.Contains(Neighbour->getTileType()))
					{
						continue;
					}

					if (Request.AllowedTiles.Num() > 0 && !Request.AllowedTiles.Contains(Neighbour->getTileType()))
					{
						continue;
					}

					int32 newMovementCost = CurrentTile->getGCost() + UTableHelper::getTileDistance(CurrentTile, Neighbour);
					if (!Request.bIgnoreWeigths)
					{
						newMovementCost += Neighbour->getMovementCost();
					}

					if (newMovementCost < Neighbour->getGCost() || !OpenSet.Contains(Neighbour))
					{
						Neighbour->GCost = newMovementCost;
						Neighbour->HCost = UTableHelper::getTileDistance(Neighbour, EndTile);

						Neighbour->PathParent = CurrentTile;

						OpenSet.Add(Neighbour);
					}
				}

			}

			MaxTries++;
			if (MaxTries >= 500)
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

	Path.Add(Start);
	
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

TArray<UTileData*> ATableWorldTable::FindPathRoad(UTileData* StartTile, UTileData* EndTile, bool bAllowDiag /*= false*/)
{
	//Find the path
	if (StartTile && EndTile) 
	{
		FVector2D Start = StartTile->getPositionAsVector();
		FVector2D End = EndTile->getPositionAsVector();

		TArray<ETileType> AllowedTiles;
		AllowedTiles.Add(ETileType::DirtRoad);

		FTablePathfindingRequest request;
		request.StartTile = Start;
		request.EndTile = End;
		request.bAllowDiag = bAllowDiag;
		request.bIgnoreWeigths = false;
		request.AllowedTiles = AllowedTiles;

		return FindPath(request);
	}

	DebugError("Couldnt find a valid path between the start and end!");
	return TArray<UTileData*>();
}

ABuildableTile* ATableWorldTable::getBuildingWithID(FName UID)
{
	if (UID == NAME_None)return nullptr;
	
	for(int32 i = 0; i < getBuildings().Num(); i++)
	{
		ABuildableTile* Tile = getBuildings()[i];
		if(Tile)
		{
			if(Tile->getUID() == UID)
			{
				return Tile;
			}
		}
	}

	return nullptr;
}

TArray<ABuildableTile*> ATableWorldTable::getBuildings()
{
	return Buildings;
}

TArray<ACityCentreTile*> ATableWorldTable::getCityCentres()
{
	TArray<ACityCentreTile*> CityCentres;
	for(int32 i = 0; i < Buildings.Num(); i++)
	{
		ABuildableTile* Building = Buildings[i];
		if(Building)
		{
			if(Building->IsA(ACityCentreTile::StaticClass()))
			{
				ACityCentreTile* CityCentre = Cast<ACityCentreTile>(Building);
				if(CityCentre)
				{
					CityCentres.Add(CityCentre);
				}
			}
		}
	}

	return CityCentres;
}

TArray<UInfluenceComponent*> ATableWorldTable::getInfluenceComponents()
{
	TArray<UInfluenceComponent*> Comps;
	
	for (int32 i = 0; i < Buildings.Num(); i++)
	{
		ABuildableTile* Building = Buildings[i];
		if (Building)
		{
			UInfluenceComponent* Comp = Cast<UInfluenceComponent>(Building->GetComponentByClass(UInfluenceComponent::StaticClass()));
			if(Comp)
			{
				Comps.Add(Comp);
			}
		}
	}

	return Comps;
}

ACityCentreTile* ATableWorldTable::getNearestCityCenter(int32 X, int32 Y)
{
	ACityCentreTile* Best = nullptr;
	
	TArray<ACityCentreTile*> Centers = getCityCentres();
	for(int32 i = 0; i < Centers.Num(); i++)
	{
		ACityCentreTile* City = Centers[i];
		if(City)
		{
			if(!Best)
			{
				Best = City;

				continue;
			}

			int32 OldDist = UTableHelper::getDistance(X, Y, Best->getCenterX(), Best->getCenterY());
			int32 NewDist = UTableHelper::getDistance(X, Y, City->getCenterX(), City->getCenterY());

			if(NewDist < OldDist)
			{
				Best = City;
			}
		}
	}

	return Best;
}

UTexture2D* ATableWorldTable::getMinimapTexture()
{
	return MinimapTexture;
}

int32 ATableWorldTable::getNewRandomSeed()
{
	return FMath::Rand();
}

int32 ATableWorldTable::getRandomSeed()
{
	if(getGamemode())
	{
		return getGamemode()->getWorldSeed();
	}

	return 0;
}

uint8 ATableWorldTable::getWorldSize()
{
	return WorldSize;
}

void ATableWorldTable::SaveData_Implementation(UTableSavegame* Savegame)
{

}
