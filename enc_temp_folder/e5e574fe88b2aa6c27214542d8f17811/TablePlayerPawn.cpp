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
#include "World/Tile/Building/CityCentreTile.h"
#include "Misc/TableHelper.h"

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

		//Offset the camera based on the drag
		AddActorWorldOffset(FVector(CameraDragDif.X, CameraDragDif.Y, 0));
		UpdateMinimapPlayerView();

		CameraDragStart = CameraDragEnd;
	}

	//Get the currently selected tile
	bool bCanGetNewHoverTile = true;

	//Convert the mouse position to the tile location
	int32 MouseX = getMouseWorldLocation().X / 100;
	int32 MouseY = getMouseWorldLocation().Y / 100;

	if (getGamemode())
	{
		//Select the tile
		SelectedTile = getGamemode()->getTile(MouseX, MouseY);
	}

	MoveSelectedBuilding();
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

void ATablePlayerPawn::MoveSelectedBuilding()
{
	//Check if we have a valid building to build
	if (HasValidBuildableTile())
	{
		if (SelectedTile)
		{
			//Move the building ghost  to the mouse tile location
			TileActor->SetActorLocation(getBuildingBuildLocation(SelectedTile));

			//Show the player if we can place the building or now
			TileActor->SetIsBlocked(CanBuild(CurrentBuilding.BuildingSize, SelectedTile));

			if (GlobalMaterialVariables)
			{
				//Move the building grid around the building
				UKismetMaterialLibrary::SetVectorParameterValue(this, GlobalMaterialVariables, "GridOrigin", TileActor->GetActorLocation());
			}

			//If we are dragging a building
			if (bIsDragBuilding)
			{
				//We are dragging a building
				EndDragTile = FVector2D(SelectedTile->getX(), SelectedTile->getY());
				if (DragTileLocation != EndDragTile)
				{
					DragTiles.Empty();
					DragTileLocation = EndDragTile;
					if (getGamemode())
					{
						if (getGamemode()->getTable())
						{
							TArray<UTileData*> Path = getGamemode()->getTable()->FindPath(StartDragTile, DragTileLocation, CurrentBuilding.BlockedTiles, false, false);
							for (int32 i = 0; i < Path.Num(); i++)
							{
								UTileData* Tile = Path[i];
								if (Tile)
								{
									DragTiles.Add(Tile->getPositionAsVector());
								}
							}

							DebugWarning("Rebuild Road! " + FString::FromInt(DragTiles.Num()));
						}
					}
				}

				for (int32 i = 0; i < DragTiles.Num(); i++)
				{
					FVector2D Loc = DragTiles[i];
					DrawDebugPoint(GetWorld(), FVector(Loc.X * 100 + 50, Loc.Y * 100 + 50, 0), 10, FColor::Red, false, 0, 0);
				}
			}
		}
	}
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

		return;
	}

	//Try to open the details panel
	if(SelectedTile)
	{
		if (getPlayerController()) 
		{
			//Try to get the building on the tileobject
			if (SelectedTile->getTileObject())
			{
				SelectedTile->DebugHighlightTile(1.0f);

				getPlayerController()->ShowBuildingInfoPanel(SelectedTile->getTileObject());
				return;
			}

			getPlayerController()->ShowBuildingInfoPanel(nullptr);
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
	if(HasValidBuildableTile())
	{
		bIsDragBuilding = false;
		DragTiles.Num();

		SetCurrentBuilding(FTableBuilding());
		return;
	}

	//Close all toolmenues
	if(getPlayerController())
	{
		getPlayerController()->SelectTool(EToolbarTools::None);
	}
}

void ATablePlayerPawn::Input_RightMouse_Released()
{
	
}

void ATablePlayerPawn::Input_ZoomIn()
{
	ZoomLerpGoal -= 10 * GetWorld()->GetDeltaSeconds();
	ZoomLerpGoal = FMath::Clamp(ZoomLerpGoal, 0.0f, 1.0f);

	UpdateMinimapPlayerView();
}

void ATablePlayerPawn::Input_ZoomOut()
{
	ZoomLerpGoal += 10 * GetWorld()->GetDeltaSeconds();
	ZoomLerpGoal = FMath::Clamp(ZoomLerpGoal, 0.0f, 1.0f);

	UpdateMinimapPlayerView();
}

void ATablePlayerPawn::Input_Forward(float v)
{
	if(v != 0)
	{
		AddMovementInput(GetActorForwardVector(), v);

		UpdateMinimapPlayerView();
	}
}

void ATablePlayerPawn::Input_Right(float v)
{
	if (v != 0)
	{
		AddMovementInput(GetActorRightVector(), v);

		UpdateMinimapPlayerView();
	}
}

void ATablePlayerPawn::PlaceBuilding(int32 X, int32 Y, FTableBuilding Data)
{
	if (getGamemode()) 
	{
		UTileData* PlaceTile = getGamemode()->getTile(X, Y);
		if (PlaceTile)
		{
			if (CanBuild(CurrentBuilding.BuildingSize, PlaceTile))
			{
				//Consume the buildcosts
				getGamemode()->ConsumeRescource(CurrentBuilding.NeededItems);
				
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
						if(getGamemode())
						{
							if(getGamemode()->getTable())
							{
								getGamemode()->getTable()->AddBuilding(PlacedTile);
							}
						}

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

void ATablePlayerPawn::TeleportPlayer(FVector NewLocation)
{
	SetActorLocation(NewLocation);

	UpdateMinimapPlayerView();
}

void ATablePlayerPawn::UpdateMinimapPlayerView()
{
	if(getPlayerController())
	{
		if(getGamemode())
		{
			if(getGamemode()->getTable())
			{
				float MaxX = getGamemode()->getTable()->ChunkSize * getGamemode()->getTable()->MaxSizeX;
				float MaxY = getGamemode()->getTable()->ChunkSize * getGamemode()->getTable()->MaxSizeY;

				float OurX = GetActorLocation().X;
				float OurY = GetActorLocation().Y;

				float AlphaX = (OurX / MaxX);
				AlphaX = FMath::Clamp(AlphaX, 0.0f, 1.0f);

				float AlphaY = (OurY / MaxY);
				AlphaY = FMath::Clamp(AlphaY, 0.0f, 1.0f);

				DebugError("XAlpha" + FString::SanitizeFloat(AlphaX) + "(" + FString::FromInt(OurX) + "/" + FString::FromInt(MaxX) + ")");
				DebugError("YAlpha" + FString::SanitizeFloat(AlphaY) + "(" + FString::FromInt(OurY) + "/" + FString::FromInt(MaxY) + ")");

				getPlayerController()->UpdateMinimapPlayerView(OurX, OurY, ZoomAlpha);
			}
		}
	}
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
		if (getGamemode())
		{
			//Check if we got the required rescources
			if(CurrentBuilding.NeededItems.Num() > 0)
			{
				if(!getGamemode()->OwnNeededItems(CurrentBuilding.NeededItems))
				{
					return false;
				}
			}
			
			//Check if the building is inside a building influence
			if (CurrentBuilding.bNeedsInfluence)
			{
				if (getGamemode()->getTable())
				{
					bool bCanBuild = false;

					TArray<ACityCentreTile*> CityCentre = getGamemode()->getTable()->getCityCentres();
					for (int32 i = 0; i < CityCentre.Num(); i++)
					{
						ACityCentreTile* City = CityCentre[i];
						if (City)
						{
							if (City->InInfluenceRange(PlaceTile->getX(), PlaceTile->getY(), BuildingSize))
							{
								bCanBuild = true;
								break;
							}
						}
					}

					if (!bCanBuild)
					{
						return false;
					}
				}
			}

			if (BuildingSize <= FVector2D(1, 1))
			{
				if (PlaceTile)
				{
					if (!CurrentBuilding.BlockedTiles.Contains(PlaceTile->getTileType()))
					{
						return PlaceTile->CanBuildOnTile();
					}
				}
			}

			if (PlaceTile)
			{
				for (int32 x = 0; x < BuildingSize.X; x++)
				{
					for (int32 y = 0; y < BuildingSize.Y; y++)
					{
						UTileData* Tile = getGamemode()->getTile(PlaceTile->getX() + x, PlaceTile->getY() + y);
						if (Tile)
						{
							if (CurrentBuilding.BlockedTiles.Contains(Tile->getTileType()))
							{
								return false;
							}

							if (!Tile->CanBuildOnTile())
							{
								return false;
							}
						}
						else
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

