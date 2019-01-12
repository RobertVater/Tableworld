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
#include "World/Tile/TileData.h"
#include "World/TableChunk.h"
#include "World/Tile/Building/BuildableTile.h"
#include "Kismet/KismetMaterialLibrary.h"

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

	ZoomLerpGoal = ZoomAlpha;
	AdjustZoom();
}

void ATablePlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Smoothly zoom to the target ZoomLevel
	ZoomAlpha = FMath::Lerp(ZoomAlpha, ZoomLerpGoal, ZoomSpeed*DeltaTime);
	AdjustZoom();

	//Camera Drag
	if(bDraggingCamera)
	{
		float DragStrength = FMath::Lerp(1.0f, 2.5f, ZoomAlpha);

		CameraDragEnd = getScreenMouseLocation();
		CameraDragDif = (CameraDragStart - CameraDragEnd) * DragStrength;

		AddActorWorldOffset(FVector(CameraDragDif.X, CameraDragDif.Y, 0));

		CameraDragStart = CameraDragEnd;
	}

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
			TileActor->SetActorLocation(getBuildingBuildLocation(SelectedTile));

			TileActor->SetIsBlocked(CanBuild(CurrentBuilding.BuildingSize, SelectedTile));

			if (GlobalMaterialVariables)
			{
				UKismetMaterialLibrary::SetVectorParameterValue(this, GlobalMaterialVariables, "GridOrigin", TileActor->getWorldCenter());
			}

			if(bIsDragBuilding)
			{
				//We are dragging a building
				EndDragTile = FVector2D(SelectedTile->getX(), SelectedTile->getY());
				if(DragTileLocation != EndDragTile)
				{
					DragTiles.Empty();
					DragTileLocation = EndDragTile;
					if(getGamemode())
					{
						if(getGamemode()->getTable())
						{
							TArray<UTileData*> PathTile = getGamemode()->getTable()->FindPath(StartDragTile, EndDragTile, CurrentBuilding.BlockedTiles, false, false);
							//Add the starttile
							UTileData* StartTile = getGamemode()->getTile((int32)StartDragTile.X, (int32)StartDragTile.Y);
							PathTile.Add(StartTile);

							for(int32 i = 0; i < PathTile.Num(); i++)
							{
								UTileData* Tile = PathTile[i];
								if(Tile)
								{
									DragTiles.Add(FVector2D(Tile->getX(), Tile->getY()));
								}
							}
						}
					}
				}

				for(int32 i = 0; i < DragTiles.Num(); i++)
				{
					FVector2D Loc = DragTiles[i];
					DrawDebugPoint(GetWorld(), FVector(Loc.X*100 + 50,Loc.Y*100 + 50,0), 10, FColor::Red, false, 0, 0);
				}
			}
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

		if (GlobalMaterialVariables)
		{
			UKismetMaterialLibrary::SetScalarParameterValue(this, GlobalMaterialVariables, "bShowGrid", 0.0f);
		}
	}
	
	if (Building.ID != NAME_None)
	{

		TileActor = GetWorld()->SpawnActor<ABuildableTile>(CurrentBuilding.TileClass, FVector::ZeroVector, FRotator::ZeroRotator);
		if (TileActor)
		{
			TileActor->SetIsGhost(Building);

			if(GlobalMaterialVariables)
			{
				UKismetMaterialLibrary::SetScalarParameterValue(this, GlobalMaterialVariables, "bShowGrid", (TileActor->getBuildGridRadius() <= 0) ? 0 : 1 );
			}

			if (GlobalMaterialVariables)
			{
				UKismetMaterialLibrary::SetScalarParameterValue(this, GlobalMaterialVariables, "Tiles", TileActor->getBuildGridRadius());
			}
		}
	}
}

void ATablePlayerPawn::Input_LeftMouse_Pressed()
{
	if (HasValidBuildableTile()) 
	{
		if (CurrentBuilding.bDragBuilding)
		{
			if (!bIsDragBuilding) 
			{
				bIsDragBuilding = true;

				if (SelectedTile)
				{
					StartDragTile = FVector2D(SelectedTile->getX(), SelectedTile->getY());
					EndDragTile = StartDragTile;
					DragTileLocation = StartDragTile;

					DragTiles.Add(StartDragTile);
				}
			}

			return;
		}

		if (SelectedTile)
		{
			PlaceBuilding(SelectedTile->getX(), SelectedTile->getY(), CurrentBuilding);
		}
	}
}

void ATablePlayerPawn::Input_LeftMouse_Released()
{
	if(bIsDragBuilding)
	{
		if (HasValidBuildableTile())
		{
			for (int32 i = 0; i < DragTiles.Num(); i++)
			{
				PlaceBuilding((int32)DragTiles[i].X, (int32)DragTiles[i].Y, CurrentBuilding);
			}
		}

		bIsDragBuilding = false;
		DragTiles.Empty();
	}
}

void ATablePlayerPawn::Input_MiddleMouse_Pressed()
{
	//Drag movement
	if(!bDraggingCamera)
	{
		if (getPlayerController()) 
		{
			bDraggingCamera = true;

			CameraDragStart = getScreenMouseLocation();
			CameraDragEnd = getScreenMouseLocation();
			CameraDragDif = FVector2D::ZeroVector;
		}
	}
}

void ATablePlayerPawn::Input_MiddleMouse_Released()
{
	bDraggingCamera = false;

	CameraDragStart = FVector2D::ZeroVector;
	CameraDragEnd = FVector2D::ZeroVector;
	CameraDragDif = FVector2D::ZeroVector;
}

void ATablePlayerPawn::Input_RightMouse_Pressed()
{
	if(TileActor)
	{
		bIsDragBuilding = false;
		DragTiles.Num();

		SetCurrentBuilding(FTableBuilding());
	}
}

void ATablePlayerPawn::Input_RightMouse_Released()
{
	
}

void ATablePlayerPawn::Input_ZoomIn()
{
	ZoomLerpGoal -= 10 * GetWorld()->GetDeltaSeconds();
	ZoomLerpGoal = FMath::Clamp(ZoomLerpGoal, 0.0f, 1.0f);
}

void ATablePlayerPawn::Input_ZoomOut()
{
	ZoomLerpGoal += 10 * GetWorld()->GetDeltaSeconds();
	ZoomLerpGoal = FMath::Clamp(ZoomLerpGoal, 0.0f, 1.0f);
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

void ATablePlayerPawn::PlaceBuilding(int32 X, int32 Y, FTableBuilding Data)
{
	if (getGamemode()) 
	{
		UTileData* PlaceTile = getGamemode()->getTile(X, Y);
		if (PlaceTile)
		{
			PlaceTile->DebugHighlightTile(1.0f);

			if (CanBuild(CurrentBuilding.BuildingSize, PlaceTile))
			{
				//Change a world tile using this building
				if (CurrentBuilding.BuildType == ETableBuildingBuildType::Tile)
				{
					FVector2D BuildingSize = CurrentBuilding.BuildingSize;
					for (int32 x = 0; x < BuildingSize.X; x++)
					{
						for (int32 y = 0; y < BuildingSize.Y; y++)
						{
							getGamemode()->SetTile(PlaceTile->getX() + x, PlaceTile->getY() + y, CurrentBuilding.BuildTileType, true);
						}
					}

					SetCurrentBuilding(CurrentBuilding);

					return;
				}

				//Spawn a copy of the ghost building
				if (CurrentBuilding.BuildType == ETableBuildingBuildType::Actor)
				{
					//Place a normal tile
					ABuildableTile* PlacedTile = GetWorld()->SpawnActor<ABuildableTile>(TileActor->GetClass(), getBuildingBuildLocation(PlaceTile), FRotator::ZeroRotator);
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
									UTileData* TileData = getGamemode()->getTile( X + x, Y + y);
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

						PlacedTile->Place(PlacedOnTiles, Data);

						//Select the same building again
						SetCurrentBuilding(CurrentBuilding);
					}
				}
			}
		}
	}
}

void ATablePlayerPawn::AdjustZoom()
{
	float NewZoom = FMath::Lerp(MinZoom, MaxZoom, ZoomAlpha);
	SpringArm->TargetArmLength = NewZoom;

	float NewPitch = FMath::Lerp(MinPitch, MaxPitch, ZoomAlpha);
	FRotator Rot = SpringArm->RelativeRotation;
	SpringArm->SetRelativeRotation(FRotator(-NewPitch, Rot.Yaw, 0));
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

bool ATablePlayerPawn::CanBuild(FVector2D BuildingSize, UTileData* PlaceTile)
{
	if(HasValidBuildableTile())
	{
		if(BuildingSize <= FVector2D(1,1))
		{
			if (!CurrentBuilding.BlockedTiles.Contains(PlaceTile->getTileType()))
			{
				return PlaceTile->CanBuildOnTile();
			}
		}

		if (getGamemode())
		{
			if (PlaceTile)
			{
				for (int32 x = 0; x < BuildingSize.X; x++)
				{
					for (int32 y = 0; y < BuildingSize.Y; y++)
					{
						UTileData* Tile = getGamemode()->getTile(PlaceTile->getX() + x, PlaceTile->getY() + y);
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

FVector2D ATablePlayerPawn::getScreenMouseLocation()
{
	if (getPlayerController())
	{
		float X, Y;
		getPlayerController()->GetMousePosition(X, Y);

		return FVector2D(X, Y);
	}

	return FVector2D::ZeroVector;
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

FVector ATablePlayerPawn::getBuildingBuildLocation(UTileData* Tile)
{
	if (Tile)
	{
		float X = Tile->getX() * 100;
		float Y = Tile->getY() * 100;
		float OffsetX = CurrentBuilding.BuildingSize.X * 50;
		float OffsetY = CurrentBuilding.BuildingSize.X * 50;

		return FVector(X + OffsetX, Y + OffsetY, 0.0f);
	}

	return FVector::ZeroVector;
}

