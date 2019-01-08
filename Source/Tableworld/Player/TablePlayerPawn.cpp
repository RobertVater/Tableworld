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
	}

	if(HasValidBuildableTile())
	{
		if (SelectedTile)
		{
			TileActor->SetActorLocation(SelectedTile->getWorldCenter());
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
	if(Building.ID != NAME_None)
	{
		CurrentBuilding = Building;

		if(TileActor)
		{
			TileActor->Destroy();
		}

		TileActor = GetWorld()->SpawnActor<ABuildableTile>(CurrentBuilding.TileClass, FVector::ZeroVector, FRotator::ZeroRotator);
		if(TileActor)
		{
			TileActor->SetIsGhost();
		}
	}
}

void ATablePlayerPawn::Input_LeftMouse_Pressed()
{
	if(CanBuild())
	{
		ABuildableTile* PlacedTile = GetWorld()->SpawnActor<ABuildableTile>(TileActor->GetClass(), SelectedTile->getWorldCenter(), FRotator::ZeroRotator);
		if (PlacedTile) 
		{
			SelectedTile->AddBuildableTile(PlacedTile);
			PlacedTile->Place(SelectedTile->getX(), SelectedTile->getY());

			//Select the same building again
			SetCurrentBuilding(CurrentBuilding);
		}
	}
}

void ATablePlayerPawn::Input_LeftMouse_Released()
{

}

void ATablePlayerPawn::Input_MiddleMouse_Pressed()
{

}

void ATablePlayerPawn::Input_MiddleMouse_Released()
{

}

void ATablePlayerPawn::Input_RightMouse_Pressed()
{
	if (getPlayerController())
	{
		FHitResult hit;
		getPlayerController()->GetHitResultUnderCursor(ECC_Visibility, false, hit);

		if (hit.bBlockingHit)
		{
			DrawDebugPoint(GetWorld(), hit.ImpactPoint, 10.0f, FColor::Orange, false, 5, 0);

			if(getGamemode())
			{
				ATableWorldTable* Table = getGamemode()->getTable();
				if(Table)
				{
					int32 x = (hit.ImpactPoint.X / 100);
					int32 y = (hit.ImpactPoint.Y / 100);

					//DrawDebugString(GetWorld(), hit.ImpactPoint, FString::FromInt(x) + "," + FString::FromInt(y), NULL, FColor::White, 1, false);
					//Table->SetTile(x, y, ETileType::Sand);

					ATableChunk* Chunk = Table->getChunkForTile(x, y);
					if(Chunk)
					{
						Chunk->ChangeTileHeight(x, y, 100.0f);
					}
				}
			}
		}
	}
	/*
	if(GetWorld()->GetFirstPlayerController())
	{
		float x, y;
		GetWorld()->GetFirstPlayerController()->GetMousePosition(x, y);

		FVector2D MouseLoc = FVector2D(x, y);

		CamRotStart = MouseLoc;
		CamRotDif = FVector2D::ZeroVector;
		CamRotEnd = MouseLoc;

		bRotateCamera = true;
	}*/
}

void ATablePlayerPawn::Input_RightMouse_Released()
{
	CamRotStart = FVector2D::ZeroVector;
	CamRotDif = FVector2D::ZeroVector;
	CamRotEnd = FVector2D::ZeroVector;

	bRotateCamera = false;
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

bool ATablePlayerPawn::CanBuild()
{
	if (SelectedTile) 
	{
		if(SelectedTile->HasTileObject())
		{
			return false;
		}

		return HasValidBuildableTile();
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

