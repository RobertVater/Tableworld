// Fill out your copyright notice in the Description page of Project Settings.

#include "BuildableTile.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/TableGamemode.h"
#include "Kismet/GameplayStatics.h"
#include "World/TableWorldTable.h"
#include "../TileData.h"
#include "Core/TableGameInstance.h"
#include "Components/InstancedStaticMeshComponent.h"

ABuildableTile::ABuildableTile()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
	TileMesh->SetCollisionProfileName("NoCollision");
	TileMesh->SetupAttachment(Root);

	GridHighlight = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GridHighlight"));
	GridHighlight->SetCastShadow(false);
	GridHighlight->SetCollisionProfileName("NoCollision");
	GridHighlight->SetGenerateOverlapEvents(false);
	GridHighlight->SetupAttachment(GetRootComponent());

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetRelativeLocation(FVector(0, 0, 50));
	CollisionBox->SetBoxExtent(FVector(50, 50, 50));
	CollisionBox->SetupAttachment(RootComponent);
}

void ABuildableTile::BeginPlay()
{
	Super::BeginPlay();
	
	DefaultMaterial = TileMesh->GetMaterial(0);
}

void ABuildableTile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABuildableTile::ShowGridRadius()
{
	ClearGridRadius();

	int32 Radius = getBuildGridRadius();

	if (Radius > 0)
	{
		if (getTable())
		{
			for (int32 x = -Radius; x <= Radius; x++)
			{
				for (int32 y = -Radius; y <= Radius; y++)
				{
					if ((x * x) + (y * y) <= (Radius * Radius))
					{
						int32 XX = (GetActorLocation().X / 100 + x);
						int32 YY = (GetActorLocation().Y / 100 + y);

						UTileData* Tile = getTable()->getTile(XX, YY);
						if (Tile)
						{
							FTransform trans;
							FVector Location = FVector(Tile->getX() * 100.0f + 50.0f, Tile->getY() * 100.0f + 50.0f, 1.0f);

							trans.SetLocation(Location);
							GridHighlight->AddInstanceWorldSpace(trans);
						}
					}
				}
			}
		}
	}
}

void ABuildableTile::ClearGridRadius()
{
	if(GridHighlight)
	{
		GridHighlight->ClearInstances();
	}
}

void ABuildableTile::SetIsGhost(FTableBuilding nBuildingData)
{
	BuildingData = nBuildingData;

	bIsGhost = true;

	TileMesh->SetRenderCustomDepth(true);
	TileMesh->SetMaterial(0, GhostMaterial);

	DynMaterial = TileMesh->CreateDynamicMaterialInstance(0);

	ShowGridRadius();
}

void ABuildableTile::Place(TArray<FVector2D> nPlacedOnTiles, FTableBuilding nBuildingData)
{
	PlacedOnTiles = nPlacedOnTiles;
	BuildingData = nBuildingData;

	if(nPlacedOnTiles.Num() > 0)
	{
		TileX = (int32)nPlacedOnTiles[0].X;
		TileY = (int32)nPlacedOnTiles[0].Y;
	}

	TileMesh->SetRenderCustomDepth(false);
	TileMesh->SetMaterial(0, DefaultMaterial);
	bIsGhost = false;

	//Grab the tiles around us
	getTilesAroundUs(true);
}

void ABuildableTile::SetIsBlocked(bool bBlocked)
{
	if(DynMaterial)
	{
		float v = bBlocked ? 0.0f : 1.0f;
		DynMaterial->SetScalarParameterValue("bBlocked", v);
	}
}

void ABuildableTile::SetHaulLocked(bool bHaulLocked)
{
	bHaulerIsComming = bHaulLocked;
}

void ABuildableTile::StartWork()
{
	bIsWorking = true;
}

void ABuildableTile::StopWork()
{
	bIsWorking = false;
}

UTileData* ABuildableTile::getValidTile()
{
	if (!getTable())return nullptr;
	
	int32 XSize = BuildingData.BuildingSize.X;
	int32 YSize = BuildingData.BuildingSize.Y;

	for(int32 SX = 0; SX < XSize; SX++)
	{
		for (int32 SY = 0; SY < YSize; SY++)
		{
			for(int32 x = -1; x <= 1; x++)
			{
				for (int32 y = -1; y <= 1; y++)
				{
					int32 CheckX = (TileX + SX) + x;
					int32 CheckY = (TileY + SY) + y;

					UTileData* Tile = getTable()->getTile(CheckX, CheckY);
					if(Tile)
					{
						Tile->DebugHighlightTile(1.0f);
						if(!Tile->IsBlocked())
						{
							return Tile;
						}
					}
				}
			}
		}
	}
	
	DebugError("Did not found a valid tile!");
	return nullptr;
}

UTileData* ABuildableTile::getTile()
{
	if(getGamemode())
	{
		return getGamemode()->getTile(TileX, TileY);
	}

	return nullptr;
}

ATableGamemode* ABuildableTile::getGamemode()
{
	if(!GM)
	{
		GM = Cast<ATableGamemode>(UGameplayStatics::GetGameMode(this));
	}

	return GM;
}

UTableGameInstance* ABuildableTile::getGameInstance()
{
	if(!GI)
	{
		GI = Cast<UTableGameInstance>(UGameplayStatics::GetGameInstance(this));
	}

	return GI;
}

ATableWorldTable* ABuildableTile::getTable()
{
	if(!ParentTable)
	{
		if(getGamemode())
		{
			ParentTable = getGamemode()->getTable();
		}
	}

	return ParentTable;
}

bool ABuildableTile::isWorking()
{
	return bIsWorking;
}

int32 ABuildableTile::getTileX()
{
	return TileX;
}

int32 ABuildableTile::getTileY()
{
	return TileY;
}

FVector2D ABuildableTile::getBuildingSize()
{
	return BuildingData.BuildingSize;
}

FVector ABuildableTile::getWorldCenter()
{
	FVector Loc = GetActorLocation();

	if(BuildingData.ID != NAME_None)
	{
		Loc.X = getTileX() * 100;
		Loc.Y = getTileY() * 100;
	}

	return Loc;
}

FTableBuilding ABuildableTile::getBuildingData()
{
	return BuildingData;
}

int32 ABuildableTile::getBuildGridRadius()
{
	return 0;
}

int32 ABuildableTile::getCurrentStorage()
{
	return 0;
}

int32 ABuildableTile::getMaxStorage()
{
	return 0;
}

float ABuildableTile::getHaulTreshold()
{
	return 0.55f;
}

TArray<FProductionItem> ABuildableTile::getInputItems()
{
	return TArray<FProductionItem>();
}

TArray<FProductionItem> ABuildableTile::getOutputItems()
{
	return TArray<FProductionItem>();
}

FColor ABuildableTile::getMinimapColor()
{
	return FColor::Red;
}

TArray<UTileData*> ABuildableTile::getTilesAroundUs(bool bForceRegenerate)
{
	if(bForceRegenerate)
	{
		TilesAroundUs.Empty();
		for (int32 SX = 0; SX < (int32)BuildingData.BuildingSize.X; SX++)
		{
			for (int32 SY = 0; SY < (int32)BuildingData.BuildingSize.Y; SY++)
			{
				for (int32 x = -1; x <= 1; x++)
				{
					for (int32 y = -1; y <= 1; y++)
					{
						int32 CheckX = (TileX + SX) + x;
						int32 CheckY = (TileY + SY) + y;

						UTileData* Tile = getTable()->getTile(CheckX, CheckY);
						if (Tile)
						{
							if (!Tile->HasTileObject())
							{
								Tile->DebugHighlightTile(1.0f);
								TilesAroundUs.Add(Tile);
							}
						}
					}
				}
			}
		}
	}
	
	return TilesAroundUs;
}

bool ABuildableTile::isHaulerComming()
{
	return bHaulerIsComming;
}
