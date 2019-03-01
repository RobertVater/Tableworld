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
#include "DrawDebugHelpers.h"
#include "World/TableChunk.h"
#include "Savegame/TableSavegame.h"
#include "Component/WorkerComponent.h"

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
	CollisionBox->SetCollisionProfileName("NoCollision");
	CollisionBox->SetupAttachment(RootComponent);
}

void ABuildableTile::BeginPlay()
{
	Super::BeginPlay();
	
	DefaultMaterial = TileMesh->GetMaterial(0);

	CollisionBox->SetHiddenInGame(!UTableHelper::isDebug());
}

void ABuildableTile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABuildableTile::BuildGrid(int32 Radius)
{
	ClearGridRadius();

	if (Radius > 0)
	{
		if (getTable())
		{
			for (int32 x = -Radius; x <= Radius; x++)
			{
				for (int32 y = -Radius; y <= Radius; y++)
				{
					if ((x * x) + (y * y) < (Radius * Radius))
					{
						int32 XX = ((GetActorLocation().X / 100.0f) + x);
						int32 YY = ((GetActorLocation().Y / 100.0f) + y);

						UTileData* Tile = getTable()->getTile(XX, YY);
						if (Tile)
						{
							FTransform trans;
							FVector Location = Tile->getWorldCenter();
							Location.Z = (Tile->getHeigth() * MapGenerator::TileSize) + getGridHeigth();

							trans.SetLocation(Location);
							GridHighlight->AddInstanceWorldSpace(trans);
						}
					}
				}
			}
		}
	}
}

void ABuildableTile::ShowGridRadius()
{
	BuildGrid(getBuildGridRadius());
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
	GhostOffsetZ = 50.0f;

	if(getDynMaterial())
	{
		getDynMaterial()->SetScalarParameterValue("bIsGhost", 1.0f);
		getDynMaterial()->SetVectorParameterValue("BlockedColor", FColor::Red);
	}

	DynGridMaterial = GridHighlight->CreateDynamicMaterialInstance(0);
	if(DynGridMaterial)
	{
		DynGridMaterial->SetVectorParameterValue("Color", getGridColor());
	}

	ShowGridRadius();
}

void ABuildableTile::SetHighlighted(bool bHighLight)
{
	if (!CanBeDeleted())return;
	
	if(getDynMaterial())
	{
		getDynMaterial()->SetScalarParameterValue("bIsGhost", bHighLight ? 1.0f : 0.0f);
		getDynMaterial()->SetVectorParameterValue("BlockedColor", FColor::White);
	}
}

void ABuildableTile::Place(FVector PlaceLoc, TArray<FVector2D> nPlacedOnTiles, FTableBuilding nBuildingData, bool bNewRotated, bool bLoadBuilding)
{
	InitBuilding(PlaceLoc, nPlacedOnTiles, nBuildingData, bNewRotated);
}

void ABuildableTile::InitBuilding(FVector PlaceLoc, TArray<FVector2D> nPlacedOnTiles, FTableBuilding nBuildingData, bool bNewRotated)
{
	bRotated = bNewRotated;

	//Generate a UID
	UID = FName(*FGuid::NewGuid().ToString());

	//Snap the building to the place location
	SetActorLocation(PlaceLoc);

	//Reset the rotation of the building
	SetActorRotation(FRotator(0, GetActorRotation().Yaw, 0));


	PlacedOnTiles = nPlacedOnTiles;
	BuildingData = nBuildingData;

	if (DynMaterial)
	{
		DynMaterial->SetScalarParameterValue("bIsGhost", 0.0f);
	}

	if (nPlacedOnTiles.Num() > 0)
	{
		TileX = (int32)nPlacedOnTiles[0].X;
		TileY = (int32)nPlacedOnTiles[0].Y;
	}

	if (bChangeFoundationTiles)
	{
		if (getTable())
		{
			TArray<UTileData*> Tiles;
			int32 HalfX = (int32)getBuildingSize().X / 2;
			int32 HalfY = (int32)getBuildingSize().Y / 2;

			DebugWarning(FString::FromInt(HalfX) + "x" + FString::FromInt(HalfY));

			for (int32 x = -HalfX; x <= HalfX; x++)
			{
				for (int32 y = -HalfY; y <= HalfY; y++)
				{
					int32 X = getCenterX() + x;
					int32 Y = getCenterY() + y;

					UTileData* Tile = getTable()->getTile(X, Y);
					if (Tile)
					{
						getTable()->SetTile(X, Y, FoundationTile);
						Tiles.Add(Tile);
					}
				}
			}

			getTable()->SetMultipleTiles(Tiles);
		}
	}


	bIsGhost = false;

	//Grab the tiles around us
	getTilesAroundUs(true);
}

void ABuildableTile::MoveBuildingToLocation(FVector NewLoc)
{
	if (bIsGhost)
	{
		float Delta = GetWorld()->GetDeltaSeconds();
		float LerpSpeed = 15.0f;
		float WobbleSpeed = 20.0f;

		//Wobble
		float WobbleMax = 5.0f;
		float WobbleRoll = FMath::Clamp((GetActorLocation().Y - NewLoc.Y), -WobbleMax, WobbleMax);
		float WobblePitch = FMath::Clamp((GetActorLocation().X - NewLoc.X), -WobbleMax, WobbleMax);

		float WobbleLerpRoll = FMath::Lerp(GetActorRotation().Roll, WobbleRoll, WobbleSpeed * Delta);
		float WobbleLerpPitch = FMath::Lerp(GetActorRotation().Pitch, WobblePitch, WobbleSpeed * Delta);

		SetActorRotation(FRotator(WobbleLerpPitch, GetActorRotation().Yaw, -WobbleLerpRoll));

		float LerpLocationX = FMath::Lerp(GetActorLocation().X, NewLoc.X, LerpSpeed * Delta);
		float LerpLocationY = FMath::Lerp(GetActorLocation().Y, NewLoc.Y, LerpSpeed * Delta);

		SetActorLocation(FVector(LerpLocationX, LerpLocationY, GhostOffsetZ));
	}
}

void ABuildableTile::SetIsBlocked(bool bBlocked)
{
	if(DynMaterial)
	{
		DynMaterial->SetVectorParameterValue("BlockedColor", bBlocked ? FColor::Green : FColor::Red);
	}
}

void ABuildableTile::SetHaulLocked(bool bHaulLocked)
{
	bHaulerIsComming = bHaulLocked;
}

void ABuildableTile::OnBuildingRemoved()
{
	if (getGamemode()) 
	{
		if (getGamemode()->getTable())
		{
			//Get the nearest city center
			ACityCentreTile* BestCity = getGamemode()->getTable()->getNearestCityCenter(getCenterX(), getCenterY());

			//Refund half of the buildings cost
			for (int32 i = 0; i < getBuildingData().NeededItems.Num(); i++)
			{
				FNeededItems Item = getBuildingData().NeededItems[i];

				int32 Refund = Item.NeededAmount / 2;

				getGamemode()->AddFloatingItem(Item.NeededItem, Refund, GetActorLocation());

				getGamemode()->ModifyRescource(Item.NeededItem, Refund);

				if(BestCity)
				{
					BestCity->ModifyInventory(Item.NeededItem, Refund);
				}
			}

			//Remove the building from the building list
			getGamemode()->getTable()->RemoveBuilding(this);
		}

		//Check if this building has a worker component.
		if (GetComponentByClass(UWorkerComponent::StaticClass()))
		{
			UWorkerComponent* WorkerComponent = Cast<UWorkerComponent>(GetComponentByClass(UWorkerComponent::StaticClass()));
			if (WorkerComponent)
			{
				WorkerComponent->OnBuildingDestroyed();
			}
		}

		TArray<UTileData*> Tiles;

		//Clear our placed tiles
		for (int32 i = 0; i < PlacedOnTiles.Num(); i++)
		{
			FVector2D Location = PlacedOnTiles[i];

			int32 X = (int32)Location.X;
			int32 Y = (int32)Location.Y;
			
			UTileData* Tile = getGamemode()->getTile(X, Y);
			if (Tile)
			{
				Tile->ClearBuildingTile();

				ETileType Type = Tile->getPreviousTileType();
				getGamemode()->getTable()->SetTile(Tile->getX(), Tile->getY(), Type);

				Tiles.Add(getGamemode()->getTile(Tile->getX(),Tile->getY()));
			}
		}

		getGamemode()->getTable()->SetMultipleTiles(Tiles);

		//Destroy the building
		Destroy();
	}
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
	
	int32 XSize = getBuildingSize().X;
	int32 YSize = getBuildingSize().Y;

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

int32 ABuildableTile::getCenterX()
{
	int32 HalfX = ((int32)getBuildingSize().X / 2);

	if (HalfX > 0) 
	{
		return getTileX() + HalfX;
	}

	return getTileX();
}

int32 ABuildableTile::getCenterY()
{
	int32 HalfY = ((int32)getBuildingSize().Y / 2);

	if (HalfY > 0)
	{
		return getTileY() + HalfY;
	}

	return getTileY();
}

FVector2D ABuildableTile::getBuildingSize()
{
	if(bRotated)
	{
		return FVector2D(BuildingData.BuildingSize.Y, BuildingData.BuildingSize.X);
	}
	
	return BuildingData.BuildingSize;
}

FVector ABuildableTile::getWorldCenter()
{
	FVector Loc = GetActorLocation();

	if(BuildingData.ID != NAME_None)
	{
		if (getBuildingSize() <= FVector2D(1, 1))
		{
			Loc.X = getTileX() * 100 + 50;
			Loc.Y = getTileY() * 100 + 50;

			return Loc;
		}

		int32 HalfX = (int32)getBuildingSize().X / 2;
		int32 HalfY = (int32)getBuildingSize().Y / 2;

		Loc.X = (getTileX() + HalfX) * 100 + 50;
		Loc.Y = (getTileY() + HalfY) * 100 + 50;

		return Loc;
	}

	return Loc;
}

FTableBuilding ABuildableTile::getBuildingData()
{
	return BuildingData;
}

TArray<UTileData*> ABuildableTile::getPlacedOnTiles()
{
	TArray<UTileData*> Tiles;

	for (int32 i = 0; i < PlacedOnTiles.Num(); i++)
	{
		FVector2D Location = PlacedOnTiles[i];

		int32 X = (int32)Location.X;
		int32 Y = (int32)Location.Y;

		UTileData* Tile = getGamemode()->getTile(X, Y);
		if (Tile)
		{
			Tiles.Add(Tile);
		}
	}

	return Tiles;
}

bool ABuildableTile::CanBeDeleted()
{
	return true;
}

int32 ABuildableTile::getBuildGridRadius()
{
	return 0;
}

FColor ABuildableTile::getGridColor()
{
	return FColor::Red;
}

float ABuildableTile::getGridHeigth()
{
	return 1.0f;
}

bool ABuildableTile::requiresInfluence()
{
	if(BuildingData.ID != NAME_None)
	{
		return BuildingData.bNeedsInfluence;
	}

	return false;
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

FName ABuildableTile::getUID()
{
	return UID;
}

TArray<UTileData*> ABuildableTile::getTilesAroundUs(bool bForceRegenerate)
{
	if(bForceRegenerate)
	{
		bIsConnectedToRoad = false;
		
		TilesAroundUs.Empty();
		for (int32 SX = 0; SX < (int32)getBuildingSize().X; SX++)
		{
			for (int32 SY = 0; SY < (int32)getBuildingSize().Y; SY++)
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
								TilesAroundUs.Add(Tile);
							}

							if(Tile->getTileType() == ETileType::DirtRoad)
							{
								bIsConnectedToRoad = true;
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

bool ABuildableTile::isConnectedToRoad()
{
	return bIsConnectedToRoad;
}

UMaterialInstanceDynamic* ABuildableTile::getDynMaterial()
{
	if(!DynMaterial)
	{
		DynMaterial = TileMesh->CreateDynamicMaterialInstance(0);
	}

	return DynMaterial;
}

void ABuildableTile::SaveData_Implementation(UTableSavegame* Savegame)
{
	
}

FTableInfoPanel ABuildableTile::getInfoPanelData_Implementation()
{
	FTableInfoPanel Panel;

	FTableInfo_Text NameText;
	NameText.RawText = getBuildingData().Name;
	NameText.Icon = getBuildingData().Icon;
	NameText.Size = 25;

	Panel.DetailText.Add(NameText);

	Panel.WorldContext = this;
	return Panel;
}

FTableInfoPanel ABuildableTile::getUpdateInfoPanelData_Implementation()
{
	FTableInfoPanel Data;

	return Data;
}
