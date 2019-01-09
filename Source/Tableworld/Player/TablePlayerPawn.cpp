// Fill out your copyright notice in the Description page of Project Settings.

#include "TablePlayerPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Tableworld.h"
#include "TablePlayerController.h"
#include "Core/TableGamemode.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "World/TableWorldTable.h"
#include "World/Tile/BuildableTile.h"
#include "World/Tile/TileData.h"
#include "World/TableChunk.h"

ATablePlayerPawn::ATablePlayerPawn()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->TargetArmLength = 2000.0f;
	SpringArm->bDoCollisionTest = false;
	SpringArm->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	PrimaryActorTick.bCanEverTick = true;

}

void ATablePlayerPawn::BeginPlay()
{
	Super::BeginPlay();

}

void ATablePlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Get the currently selected tile
	bool bCanGetNewHoverTile = true;

	int32 MouseX = getMouseWorldLocation().X / 100;
	int32 MouseY = getMouseWorldLocation().Y / 100;

	if(SelectedTile)
	{
		int32 TX = SelectedTile->getX();
		int32 TY = SelectedTile->getY();

		if(MouseX == TX && MouseY == TY)
		{
			bCanGetNewHoverTile = false;
		}
	}

	if(bCanGetNewHoverTile)
	{
		if(getGamemode())
		{
			SelectedTile = getGamemode()->getTile(MouseX, MouseY);
		}
	}

	//Highlite the tile we selected
	if(SelectedTile)
	{
		DrawDebugBox(GetWorld(), FVector(SelectedTile->getWorldX() + 50, SelectedTile->getWorldY() + 50, 0), FVector(50, 50, 10), FColor::Red, false, 0, 0, 2.0f);

		DrawDebugString(GetWorld(), FVector(SelectedTile->getWorldX() + 50, SelectedTile->getWorldY() + 50, 0), FString::FromInt((uint8)SelectedTile->getTileType()), NULL, FColor::White, 0, true);
		DrawDebugString(GetWorld(), FVector(SelectedTile->getWorldX() + 50, SelectedTile->getWorldY() + 50 + 20, 0), FString::FromInt((uint8)SelectedTile->getTileRescources()), NULL, FColor::White, 0, true);
	}

	if(HasValidBuildableTile())
	{
		if (SelectedTile)
		{
			TileActor->SetActorLocation(getBuildingBuildLocation());

			TileActor->SetIsBlocked(CanBuild(CurrentBuilding.BuildingSize));
		}
	}
}

void ATablePlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("Forward", this, &ATablePlayerPawn::Input_Forward);
	PlayerInputComponent->BindAxis("Right", this, &ATablePlayerPawn::Input_Right);

	PlayerInputComponent->BindAction("ZoomIn", IE_Pressed, this, &ATablePlayerPawn::Input_ZoomIn);
	PlayerInputComponent->BindAction("ZoomOut", IE_Pressed, this, &ATablePlayerPawn::Input_ZoomOut);


	PlayerInputComponent->BindAction("LeftMouseClick", IE_Pressed, this, &ATablePlayerPawn::Input_LeftMouse_Pressed);
	PlayerInputComponent->BindAction("LeftMouseClick", IE_Released, this, &ATablePlayerPawn::Input_LeftMouse_Released);

	PlayerInputComponent->BindAction("RightMouseClick", IE_Pressed, this, &ATablePlayerPawn::Input_RightMouse_Pressed);
	PlayerInputComponent->BindAction("RightMouseClick", IE_Released, this, &ATablePlayerPawn::Input_RightMouse_Released);

	PlayerInputComponent->BindAction("MiddleMouseClick", IE_Pressed, this, &ATablePlayerPawn::Input_MiddleMouse_Pressed);
	PlayerInputComponent->BindAction("MiddleMouseClick", IE_Released, this, &ATablePlayerPawn::Input_MiddleMouse_Released);
}

void ATablePlayerPawn::SetCurrentBuilding(FTableBuilding Building)
{
	CurrentBuilding = Building;

	if (TileActor)
	{
		TileActor->Destroy();
		TileActor = nullptr;
	}
	
	if (Building.ID != NAME_None)
	{
		TileActor = GetWorld()->SpawnActor<ABuildableTile>(CurrentBuilding.TileClass, FVector::ZeroVector, FRotator::ZeroRotator);
		if (TileActor)
		{
			TileActor->SetIsGhost();
		}
	}
}

void ATablePlayerPawn::Input_LeftMouse_Pressed()
{
	if (CurrentBuilding.bDragBuilding)
	{
		bIsDragBuilding = true;
		return;
	}

	if (CanBuild(CurrentBuilding.BuildingSize))
	{
		if(CurrentBuilding.BuildType == ETableBuildingBuildType::Tile)
		{
			if (SelectedTile)
			{
				if (getGamemode())
				{
					FVector2D BuildingSize = CurrentBuilding.BuildingSize;
					for (int32 x = 0; x < BuildingSize.X; x++)
					{
						for (int32 y = 0; y < BuildingSize.Y; y++)
						{
							getGamemode()->SetTile(SelectedTile->getX(), SelectedTile->getY(), CurrentBuilding.BuildTileType, true);
						}
					}

					//getGamemode()->SetTile(SelectedTile->getX(), SelectedTile->getY(), ETileType::Sand, true);
					SetCurrentBuilding(CurrentBuilding);
				}
			}
			return;
		}
		
		if (CurrentBuilding.BuildType == ETableBuildingBuildType::Actor) 
		{
			//Place a normal tile
			ABuildableTile* PlacedTile = GetWorld()->SpawnActor<ABuildableTile>(TileActor->GetClass(), getBuildingBuildLocation(), FRotator::ZeroRotator);
			if (PlacedTile)
			{
				TArray<FVector2D> PlacedOnTiles;

				FVector2D BuildingSize = CurrentBuilding.BuildingSize;
				if (BuildingSize > FVector2D(1, 1))
				{
					for (int32 x = 0; x < BuildingSize.X; x++)
					{
						for (int32 y = 0; y < BuildingSize.Y; y++)
						{
							UTileData* TileData = getGamemode()->getTile(SelectedTile->getX() + x, SelectedTile->getY() + y);
							if (TileData)
							{
								TileData->AddBuildableTile(PlacedTile);
								PlacedOnTiles.Add(FVector2D(TileData->getX(), TileData->getY()));
							}
						}
					}
				}
				else
				{
					//The building is a 1 by 1
					PlacedOnTiles.Add(FVector2D(SelectedTile->getX(), SelectedTile->getY()));
					SelectedTile->AddBuildableTile(PlacedTile);
				}

				PlacedTile->Place(PlacedOnTiles);

				//Select the same building again
				SetCurrentBuilding(CurrentBuilding);
			}
		}
	}
}

void ATablePlayerPawn::Input_LeftMouse_Released()
{
	if(bIsDragBuilding)
	{
		if(DragTileActors.Num() > 0)
		{
			
		}
	}
}

void ATablePlayerPawn::Input_MiddleMouse_Pressed()
{

}

void ATablePlayerPawn::Input_MiddleMouse_Released()
{

}

void ATablePlayerPawn::Input_RightMouse_Pressed()
{
	if(TileActor)
	{
		SetCurrentBuilding(FTableBuilding());
	}
}

void ATablePlayerPawn::Input_RightMouse_Released()
{
	
}

void ATablePlayerPawn::Input_ZoomIn()
{
	ZoomAlpha -= 10 * GetWorld()->GetDeltaSeconds();
	ZoomAlpha = FMath::Clamp(ZoomAlpha, 0.0f, 1.0f);

	DebugWarning("ZoomAlpha: " + FString::SanitizeFloat(ZoomAlpha));
	AdjustZoom();
}

void ATablePlayerPawn::Input_ZoomOut()
{
	ZoomAlpha += 10 * GetWorld()->GetDeltaSeconds();
	ZoomAlpha = FMath::Clamp(ZoomAlpha, 0.0f, 1.0f);

	AdjustZoom();
}

void ATablePlayerPawn::Input_Forward(float v)
{
	if(v != 0)
	{
		AddMovementInput(GetActorForwardVector(), v);
	}
}

void ATablePlayerPawn::Input_Right(float v)
{
	if (v != 0)
	{
		AddMovementInput(GetActorRightVector(), v);
	}
}

void ATablePlayerPawn::AdjustZoom()
{
	float NewZoom = FMath::Lerp(MinZoom, MaxZoom, ZoomAlpha);

	DebugWarning("Zoom: " + FString::SanitizeFloat(NewZoom));
	SpringArm->TargetArmLength = NewZoom;
}

bool ATablePlayerPawn::HasValidBuildableTile()
{
	if(CurrentBuilding.ID != NAME_None)
	{
		if(TileActor)
		{
			return true;
		}
	}

	return false;
}

bool ATablePlayerPawn::CanBuild(FVector2D BuildingSize)
{
	if(HasValidBuildableTile())
	{
		if(BuildingSize <= FVector2D(1,1))
		{
			if (!CurrentBuilding.BlockedTiles.Contains(SelectedTile->getTileType()))
			{
				return SelectedTile->CanBuildOnTile();
			}
		}

		if (getGamemode())
		{
			if (SelectedTile)
			{
				for (int32 x = 0; x < BuildingSize.X; x++)
				{
					for (int32 y = 0; y < BuildingSize.Y; y++)
					{
						UTileData* Tile = getGamemode()->getTile(SelectedTile->getX() + x, SelectedTile->getY() + y);
						if(Tile)
						{
							if(CurrentBuilding.BlockedTiles.Contains(Tile->getTileType()))
							{
								return false;
							}
							
							if(!Tile->CanBuildOnTile())
							{
								return false;
							}
						}else
						{
							return false;
						}
					}
				}

				return true;
			}
		}
	}

	return false;
}

ATableGamemode* ATablePlayerPawn::getGamemode()
{
	if(!GM)
	{
		GM = Cast<ATableGamemode>(UGameplayStatics::GetGameMode(this));
	}

	return GM;
}

ATablePlayerController* ATablePlayerPawn::getPlayerController()
{
	if(!PC)
	{
		PC = Cast<ATablePlayerController>(GetWorld()->GetFirstPlayerController());
	}

	return PC;
}

FVector ATablePlayerPawn::getMouseWorldLocation()
{
	if(getPlayerController())
	{
		FVector WorldLoc;
		FVector WorldDir;
		getPlayerController()->DeprojectMousePositionToWorld(WorldLoc, WorldDir);

		float T = 0;
		FVector HitLocation;

		UKismetMathLibrary::LinePlaneIntersection_OriginNormal(WorldLoc, WorldLoc + (WorldDir * 50000), GetActorLocation(), FVector(0, 0, 1), T, HitLocation);

		return HitLocation;
	}

	return FVector::ZeroVector;
}

FVector ATablePlayerPawn::getMouseWorldLocationGrid()
{
	FVector Loc = getMouseWorldLocation();

	Loc.X = UKismetMathLibrary::GridSnap_Float(Loc.X, 50) + 50;
	Loc.Y = UKismetMathLibrary::GridSnap_Float(Loc.Y, 50) + 50;

	return FVector(Loc.X, Loc.Y, 0.0f);
}

FVector ATablePlayerPawn::getBuildingBuildLocation()
{
	float X = SelectedTile->getX() * 100;
	float Y = SelectedTile->getY() * 100;
	float OffsetX = CurrentBuilding.BuildingSize.X * 50;
	float OffsetY = CurrentBuilding.BuildingSize.X * 50;

	return FVector(X + OffsetX, Y + OffsetY, 0.0f);
}

