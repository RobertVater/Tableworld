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
#include "World/Tile/Building/CityCentreBuilding.h"
#include "Misc/TableHelper.h"
#include "Core/TableGameInstance.h"
#include "World/Tile/Building/HarvesterBuilding.h"
#include "World/MapGenerator.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Component/InfluenceComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/AudioComponent.h"
#include "Components/InstancedStaticMeshComponent.h"

ATablePlayerPawn::ATablePlayerPawn()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->TargetArmLength = 2000.0f;
	SpringArm->bDoCollisionTest = false;
	SpringArm->SetupAttachment(GetRootComponent());

	CameraGoal = CreateDefaultSubobject<UBillboardComponent>(TEXT("CameraGoal"));
	CameraGoal->SetupAttachment(SpringArm);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetFieldOfView(90.0f);
	Camera->SetupAttachment(GetRootComponent());

	InstancedHighlightMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedHighlightMesh"));
	InstancedHighlightMesh->SetCastShadow(false);
	InstancedHighlightMesh->SetReceivesDecals(false);
	InstancedHighlightMesh->SetupAttachment(GetRootComponent());

	DragBuildingMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("DragBuilding"));
	DragBuildingMesh->SetCastShadow(false);
	DragBuildingMesh->SetReceivesDecals(false);
	DragBuildingMesh->SetupAttachment(GetRootComponent());

	HighlightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HighlightMesh"));
	HighlightMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HighlightMesh->SetCastShadow(false);
	HighlightMesh->SetupAttachment(GetRootComponent());

	PrimaryActorTick.bCanEverTick = true;
}

void ATablePlayerPawn::BeginPlay()
{
	Super::BeginPlay();

	if(SpringArm)
	{
		SpringArm->PrimaryComponentTick.bTickEvenWhenPaused = true;
	}

	PrimaryActorTick.bTickEvenWhenPaused = true;

	ZoomLerpGoal = ZoomAlpha;
	AdjustZoom();
}

void ATablePlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
		if (bDestructionmode) 
		{
			if (LastHighlightBuilding)
			{
				LastHighlightBuilding->SetHighlighted(false);
				LastHighlightBuilding = nullptr;
			}
		}
		
		//Select the tile
		SelectedTile = getGamemode()->getTile(MouseX, MouseY);

		if(SelectedTile)
		{
			if (bDestructionmode)
			{
				if (SelectedTile->getTileObject())
				{
					LastHighlightBuilding = SelectedTile->getTileObject();

					//Highlight the building
					LastHighlightBuilding->SetHighlighted(true);
				}else
				{
					FVector Loc = SelectedTile->getWorldCenter();
					Loc.Z += 100;

					if(SelectedTile->HasRescource())
					{
						Loc = SelectedTile->getRescourceTransform().GetLocation();
					}

					HighlightMesh->SetWorldLocation(Loc);
				}
			}
		}
	}

	if (!bDestructionmode) 
	{
		MoveSelectedBuilding();
	}

	UpdateCamera(DeltaTime);
}

void ATablePlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("Forward", this, &ATablePlayerPawn::Input_Forward).bExecuteWhenPaused = true;
	PlayerInputComponent->BindAxis("Right", this, &ATablePlayerPawn::Input_Right).bExecuteWhenPaused = true;

	PlayerInputComponent->BindAction("ZoomIn", IE_Pressed, this, &ATablePlayerPawn::Input_ZoomIn).bExecuteWhenPaused = true;
	PlayerInputComponent->BindAction("ZoomOut", IE_Pressed, this, &ATablePlayerPawn::Input_ZoomOut).bExecuteWhenPaused = true;


	PlayerInputComponent->BindAction("LeftMouseClick", IE_Pressed, this, &ATablePlayerPawn::Input_LeftMouse_Pressed).bExecuteWhenPaused = true;
	PlayerInputComponent->BindAction("LeftMouseClick", IE_Released, this, &ATablePlayerPawn::Input_LeftMouse_Released).bExecuteWhenPaused = true;

	PlayerInputComponent->BindAction("RightMouseClick", IE_Pressed, this, &ATablePlayerPawn::Input_RightMouse_Pressed).bExecuteWhenPaused = true;
	PlayerInputComponent->BindAction("RightMouseClick", IE_Released, this, &ATablePlayerPawn::Input_RightMouse_Released).bExecuteWhenPaused = true;

	PlayerInputComponent->BindAction("MiddleMouseClick", IE_Pressed, this, &ATablePlayerPawn::Input_MiddleMouse_Pressed).bExecuteWhenPaused = true;
	PlayerInputComponent->BindAction("MiddleMouseClick", IE_Released, this, &ATablePlayerPawn::Input_MiddleMouse_Released).bExecuteWhenPaused = true;

	//Time
	PlayerInputComponent->BindAction("GameSpeed_0", IE_Pressed, this, &ATablePlayerPawn::Input_GameSpeed_0).bExecuteWhenPaused = true;
	PlayerInputComponent->BindAction("GameSpeed_1", IE_Pressed, this, &ATablePlayerPawn::Input_GameSpeed_1).bExecuteWhenPaused = true;
	PlayerInputComponent->BindAction("GameSpeed_2", IE_Pressed, this, &ATablePlayerPawn::Input_GameSpeed_2).bExecuteWhenPaused = true;
	PlayerInputComponent->BindAction("GameSpeed_3", IE_Pressed, this, &ATablePlayerPawn::Input_GameSpeed_3).bExecuteWhenPaused = true;
	PlayerInputComponent->BindAction("GameSpeed_4", IE_Pressed, this, &ATablePlayerPawn::Input_GameSpeed_4).bExecuteWhenPaused = true;

	PlayerInputComponent->BindAction("Rotate", IE_Pressed, this, &ATablePlayerPawn::Input_Rotate).bExecuteWhenPaused = true;

}

void ATablePlayerPawn::UpdateCamera(float Delta)
{
	FVector GoalLoc = CameraGoal->GetComponentLocation();
	FRotator GoalRot = CameraGoal->GetComponentRotation();

	FVector BoxSize = FVector(50, 25, 25);

	FHitResult Hit;

	//Check if the camera would be blocked at the target lot
	if(UKismetSystemLibrary::BoxTraceSingle(this, GoalLoc, GoalLoc + FVector(0,0,1), BoxSize, GoalRot, TraceTypeQuery1, false, TArray<AActor*>(), EDrawDebugTrace::None, Hit, true))
	{
		//Get the dir
		FVector Dir = (GoalLoc - Hit.ImpactPoint);
		Dir.Normalize();

		FVector SuggestedLoc = GoalLoc + (Dir * BoxSize.X/2.0f);

		//Snap the zoom loc
		ZoomLerpGoal = (ZoomAlpha + 10.0f * Delta);

		GoalLoc = SuggestedLoc;
	}

	Camera->SetWorldLocation(GoalLoc);
	Camera->SetWorldRotation(GoalRot);

	//Smoothly zoom to the target ZoomLevel
	ZoomAlpha = FMath::Lerp(ZoomAlpha, ZoomLerpGoal, ZoomSpeed * Delta);
	AdjustZoom();
}

void ATablePlayerPawn::MoveSelectedBuilding()
{
	//Check if we have a valid building to build
	if (HasValidBuildableTile())
	{
		if (SelectedTile)
		{
			//Move the building ghost  to the mouse tile location
			BuildingGhost->MoveBuildingToLocation(getBuildingBuildLocation(SelectedTile));
			BuildingGhost->ShowGridRadius();

			//Show the player if we can place the building or now
			BuildingGhost->SetIsBlocked(CanBuild(getBuildingSize(bRotated), SelectedTile));

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
							FTablePathfindingRequest request;
							request.StartTile = StartDragTile;
							request.EndTile = DragTileLocation;
							request.ForbidenTiles = CurrentBuilding.BlockedTiles;
							request.bAllowDiag = false;
							request.bIgnoreWeigths = true;
							request.bIgnoreRescources = CurrentBuilding.bDeleteRescources;

							TArray<UTileData*> Path = getGamemode()->getTable()->FindPath(request);
							for (int32 i = 0; i < Path.Num(); i++)
							{
								UTileData* Tile = Path[i];
								if (Tile)
								{
									DragTiles.Add(Tile);
								}
							}
						}
					}
				}

				DragBuildingMesh->ClearInstances();
				InstancedHighlightMesh->ClearInstances();
				for (int32 i = 0; i < DragTiles.Num(); i++)
				{
					UTileData* Tile = DragTiles[i];
					if (Tile)
					{
						FVector2D Loc = Tile->getPositionAsVector();
						FTransform Trans = FTransform(FRotator::ZeroRotator, FVector(Loc.X * 100 + 50, Loc.Y * 100 + 50, 0));
						DragBuildingMesh->AddInstanceWorldSpace(Trans);

						//Mark Rescources that would get deleted
						if (CurrentBuilding.bDeleteRescources)
						{
							if (Tile->HadRescource())
							{
								FTransform ResTrans = Tile->getRescourceTransform();
								FVector ResLoc = ResTrans.GetLocation();

								ResTrans.SetLocation(ResLoc);

								InstancedHighlightMesh->AddInstanceWorldSpace(ResTrans);
							}
						}
					}
				}
			}
		}
	}
}

void ATablePlayerPawn::PlaySound2D(USoundBase* Sound)
{
	if(Sound)
	{
		if (!UISound) 
		{
			UISound = UGameplayStatics::SpawnSound2D(this, Sound);
			return;
		}

		if (UISound) 
		{
			UISound->SetSound(Sound);
			UISound->Play();
		}
	}
}

void ATablePlayerPawn::SetCurrentBuilding(FTableBuilding Building)
{
	CurrentBuilding = Building;

	if (BuildingGhost)
	{
		BuildingGhost->Destroy();
		BuildingGhost = nullptr;
	}
	
	if (Building.ID != NAME_None)
	{
		bRotated = false;
		BuildingGhost = GetWorld()->SpawnActor<ABuildableTile>(CurrentBuilding.TileClass, getBuildingBuildLocation(SelectedTile), FRotator::ZeroRotator);
		if (BuildingGhost)
		{
			BuildingGhost->SetIsGhost(Building);
			
			if (Building.bDragBuilding)
			{
				UStaticMesh* Mesh = BuildingGhost->TileMesh->GetStaticMesh();
				DragBuildingMesh->SetStaticMesh(Mesh);
				DragBuildingMesh->ClearInstances();
			}

			if (Building.bNeedsInfluence) 
			{
				if (getGamemode())
				{
					if (getGamemode()->getTable())
					{
						//Show influence
						getGamemode()->getTable()->ShowInfluenceGrid();
					}
				}
			}

			if(GlobalMaterialVariables)
			{
				UKismetMaterialLibrary::SetScalarParameterValue(this, GlobalMaterialVariables, "bShowGrid", (BuildingGhost->getBuildGridRadius() <= 0) ? 0 : 1 );
			}
		}
	}else
	{
		if (getGamemode())
		{
			if (getGamemode()->getTable())
			{
				//Hide influence
				getGamemode()->getTable()->HideInfluenceGrid();
			}
		}
	}
}

void ATablePlayerPawn::Input_LeftMouse_Pressed()
{
	if(bDestructionmode)
	{
		//Destroy the currently selected Tile
		if(DestroyBuilding(SelectedTile))
		{
			//Play the FX
			if (BuildingRemoveSound)
			{
				PlaySound2D(BuildingRemoveSound);
			}

			if (BuildingRemoveParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BuildingRemoveParticles, SelectedTile->getWorldCenter(), FRotator(0, FMath::RandRange(0, 360), 0));
			}
		}

		return;
	}
	
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

					DragTiles.Add(SelectedTile);
				}
			}

			return;
		}

		if (SelectedTile)
		{
			TryPlaceBuilding(SelectedTile->getX(), SelectedTile->getY(), CurrentBuilding);
		}

		return;
	}

	FVector Start = Camera->GetComponentLocation();
	FVector End = getMouseWorldLocation();

	FHitResult Hit;

	if(UKismetSystemLibrary::LineTraceSingle(this, Start, End, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), EDrawDebugTrace::None, Hit, true))
	{
		if(Hit.GetActor())
		{
			if (Hit.GetActor()->GetClass()->ImplementsInterface(UInfoPanelInterface::StaticClass()))
			{
				if (getPlayerController())
				{
					getPlayerController()->ShowInfoPanel(IInfoPanelInterface::Execute_getInfoPanelData(Hit.GetActor()));
					return;
				}
			}
		}
	}

	//Try to open the details panel
	if(SelectedTile)
	{
		if (getPlayerController()) 
		{
			//Try to get the building on the tileobject
			if (SelectedTile->getTileObject())
			{
				if (SelectedTile->getTileObject()->GetClass()->ImplementsInterface(UInfoPanelInterface::StaticClass()))
				{
					getPlayerController()->ShowInfoPanel(IInfoPanelInterface::Execute_getInfoPanelData(SelectedTile->getTileObject()));
				}
				return;
			}

			if (SelectedTile->HasRescource())
			{
				if (SelectedTile->GetClass()->ImplementsInterface(UInfoPanelInterface::StaticClass()))
				{
					getPlayerController()->ShowInfoPanel(IInfoPanelInterface::Execute_getInfoPanelData(SelectedTile));
				}
				return;
			}

			getPlayerController()->ShowInfoPanel(FTableInfoPanel());
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
				UTileData* Tile = DragTiles[i];
				if (Tile) 
				{
					TryPlaceBuilding(Tile->getX(), Tile->getY(), CurrentBuilding);
				}
			}

			InstancedHighlightMesh->ClearInstances();
			DragBuildingMesh->ClearInstances();
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

void ATablePlayerPawn::Input_Rotate()
{
	if(BuildingGhost)
	{
		bRotated = !bRotated;

		BuildingGhost->AddActorWorldRotation(FRotator(0.0f, 90.0f, 0.0f));
	}
}

void ATablePlayerPawn::Input_GameSpeed_0()
{
	ModifyGameSpeed(0);
}

void ATablePlayerPawn::Input_GameSpeed_1()
{
	ModifyGameSpeed(1);
}

void ATablePlayerPawn::Input_GameSpeed_2()
{
	ModifyGameSpeed(2);
}

void ATablePlayerPawn::Input_GameSpeed_3()
{
	ModifyGameSpeed(3);
}

void ATablePlayerPawn::Input_GameSpeed_4()
{
	ModifyGameSpeed(4);
}

void ATablePlayerPawn::ModifyGameSpeed(int32 SpeedLevel)
{
	if (getGamemode())
	{
		if(SpeedLevel <= 0)
		{
			bool bPaused = getGamemode()->StopTime();
			if(!bPaused)
			{
				//Return to the previous time
				getGamemode()->ResetTime();
			}
			return;
		}
		
		getGamemode()->ModifyTime(SpeedLevel);

		float NormalSpeed = 1.0f;
		float DeltaSpeed = NormalSpeed / SpeedLevel;

		CustomTimeDilation = DeltaSpeed;
	}
}

void ATablePlayerPawn::TryPlaceBuilding(int32 X, int32 Y, FTableBuilding Data)
{
	if (getGamemode()) 
	{
		UTileData* PlaceTile = getGamemode()->getTile(X, Y);
		if (PlaceTile)
		{
			if (CanBuild(getBuildingSize(bRotated), PlaceTile))
			{
				//Consume the buildcosts
				getGamemode()->ConsumeRescource(Data.NeededItems);
				
				ABuildableTile* PlacedBuilding = PlaceBuilding(PlaceTile, BuildingGhost->GetActorRotation().Yaw, Data);

				//Play the FX
				if(BuildingPlaceSound)
				{
					PlaySound2D(BuildingPlaceSound);
				}

				if(BuildingPlaceParticles)
				{
					if (PlacedBuilding)
					{
						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BuildingPlaceParticles, PlacedBuilding->getWorldCenter(), FRotator(0, FMath::RandRange(0, 360), 0));
					}
				}

				//Select the same building again
				SetCurrentBuilding(Data);
			}
		}
	}
}

ABuildableTile* ATablePlayerPawn::PlaceBuilding(UTileData* TargetTile, float Rotation, FTableBuilding BuildingData)
{
	if (!TargetTile)return nullptr;

	int32 X = TargetTile->getX();
	int32 Y = TargetTile->getY();
	
	//Change a world tile using this building
	if (BuildingData.BuildType == ETableBuildingBuildType::Tile)
	{
		FVector2D BuildingSize = getBuildingSize(bRotated);
		TArray<UTileData*> TilesToModify;
		for (int32 x = 0; x < BuildingSize.X; x++)
		{
			for (int32 y = 0; y < BuildingSize.Y; y++)
			{
				UTileData* Tile = getGamemode()->getTile(TargetTile->getX() + x, TargetTile->getY() + y);
				if (Tile) 
				{
					Tile->DebugHighlightTile();

					if (BuildingData.bDeleteRescources)
					{
						if (Tile->getParentChunk())
						{
							Tile->getParentChunk()->RemoveRescource(Tile);
						}
					}

					getGamemode()->SetTile(TargetTile->getX() + x, TargetTile->getY() + y, BuildingData.BuildTileType, false, true);


					Tile->LowerGrass();
					TilesToModify.Add(Tile);
				}
			}
		}

		if (getGamemode()->getTable())
		{
			getGamemode()->getTable()->SetMultipleTiles(TilesToModify);
		}

		return nullptr;
	}

	//Spawn a copy of the ghost building
	if (BuildingData.BuildType == ETableBuildingBuildType::Actor)
	{
		//Place a normal tile
		ABuildableTile* PlacedTile = GetWorld()->SpawnActor<ABuildableTile>(BuildingData.TileClass, getBuildingBuildLocation(TargetTile), FRotator(0.0f, Rotation, 0.0f));
		if (PlacedTile)
		{
			TArray<FVector2D> PlacedOnTiles;
			TArray<UTileData*> ModifiedTiles;

			FVector2D BuildingSize = getBuildingSize(bRotated);
			if (BuildingSize > FVector2D(1, 1))
			{
				int32 HalfX = (int32)BuildingSize.X / 2;
				int32 HalfY = (int32)BuildingSize.Y / 2;

				for (int32 x = -HalfX; x <= HalfX; x++)
				{
					for (int32 y = -HalfY; y <= HalfY; y++)
					{
						UTileData* TileData = getGamemode()->getTile(X + x, Y + y);
						if (TileData)
						{
							TileData->AddBuildableTile(PlacedTile);
							PlacedOnTiles.Add(FVector2D(TileData->getX(), TileData->getY()));

							ModifiedTiles.Add(TileData);
						}
					}
				}
			}
			else
			{
				//The building is a 1 by 1
				PlacedOnTiles.Add(FVector2D(TargetTile->getX(), TargetTile->getY()));
				TargetTile->AddBuildableTile(PlacedTile);

				ModifiedTiles.Add(TargetTile);
			}

			PlacedTile->Place(getBuildingBuildLocation(TargetTile), PlacedOnTiles, BuildingData, bRotated);
			if (getGamemode())
			{
				if (getGamemode()->getTable())
				{
					getGamemode()->getTable()->AddBuilding(PlacedTile);
					getGamemode()->getTable()->UpdateMinimap(ModifiedTiles);
				}
			}
			
			return PlacedTile;
		}
	}

	return nullptr;
}

void ATablePlayerPawn::ActivateDestroyMode(bool bnDestroyMode)
{
	bDestructionmode = bnDestroyMode;
	HighlightMesh->SetVisibility(bDestructionmode);

	if(!bDestructionmode)
	{
		ClearHighlight();
		
		if(LastHighlightBuilding)
		{
			LastHighlightBuilding->SetHighlighted(false);
			LastHighlightBuilding = nullptr;
		}
	}
}

int32 ATablePlayerPawn::AddHighlight(FVector Location)
{
	Location.Z += 100;

	FTransform Trans;
	Trans.SetLocation(Location);

	FVector Scale = FVector(1.0f, 1.0f, 2.0f);
	Trans.SetScale3D(Scale);

	return InstancedHighlightMesh->AddInstanceWorldSpace(Trans);
}

void ATablePlayerPawn::RemoveHighlight(int32 Index)
{
	InstancedHighlightMesh->RemoveInstance(Index);
}

void ATablePlayerPawn::ClearHighlight()
{
	InstancedHighlightMesh->ClearInstances();
}

bool ATablePlayerPawn::DestroyBuilding(UTileData* Tile)
{
	if(Tile)
	{
		//Check if we want to remove a building
		if(Tile->getTileObject())
		{
			ABuildableTile* Building = Tile->getTileObject();
			if(Building)
			{
				if (Building->CanBeDeleted()) 
				{
					Building->OnBuildingRemoved();
					return true;
				}

				return false;
			}
		}

		//Check if we want to remove the road
		if(Tile->getTileType() == ETileType::DirtRoad)
		{
			//Set the tile back to the previous tiletype
			getGamemode()->SetTile(Tile->getX(), Tile->getY(), Tile->getPreviousTileType(), true, true);
			return true;
		}

		//Check if the tile has rescources we could remove
		if(Tile->getTileRescources() != ETileRescources::None)
		{
			if (!Tile->HasHarvester()) 
			{
				ATableChunk* TileChunk = Tile->getParentChunk();
				if (TileChunk)
				{
					return TileChunk->RemoveRescource(Tile);
				}
			}
		}
	}

	return false;
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

void ATablePlayerPawn::SetZoomAlpha(float nZoomAlpha)
{
	ZoomAlpha = nZoomAlpha;
	ZoomLerpGoal = nZoomAlpha;
	AdjustZoom();
}

void ATablePlayerPawn::UpdateMinimapPlayerView()
{
	if(getPlayerController())
	{
		if(getGamemode())
		{
			if(getGamemode()->getTable())
			{
				float WorldSize = getGamemode()->getTable()->getWorldSize();
				float Scale = 8.0f / WorldSize;
				float WorldScale = WorldSize / 8.0f;

				float RectSizeX = 64 * Scale;
				float RectSizeY = 32 * Scale;
				
				float RectHalfX = RectSizeX / 2;
				float RectHalfY = RectSizeY / 2;
				
				float ScaleFactor = 100 * WorldScale;
				float OurX = ((GetActorLocation().X / ScaleFactor) * 2);
				float OurY = ((GetActorLocation().Y / ScaleFactor) * 2);

				float MapMaxX = 256.0f;
				float MapMaxY = 256.0f;

				float MapMinX = 0;
				float MapMinY = 0;

				OurX = FMath::Clamp(OurX, MapMinX, MapMaxX);
				OurY = FMath::Clamp(OurY, MapMinY, MapMaxY);

				float PlayerScale = FMath::Lerp(0.5f, 1.0f, ZoomLerpGoal) * Scale;

				getPlayerController()->UpdateMinimapPlayerView(OurX, OurY, ZoomLerpGoal, PlayerScale, WorldScale);
			}
		}
	}
}

bool ATablePlayerPawn::HasValidBuildableTile()
{
	if(CurrentBuilding.ID != NAME_None)
	{
		if(BuildingGhost)
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
			if(!PlaceTile)
			{
				return false;
			}

			if(BuildingSize < FVector2D(1,1))
			{
				return false;
			}
			
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

					TArray<UInfluenceComponent*> InfluenceComponents = getGamemode()->getTable()->getInfluenceComponents();
					for (int32 i = 0; i < InfluenceComponents.Num(); i++)
					{
						UInfluenceComponent* Influence = InfluenceComponents[i];
						if (Influence)
						{
							if (getGamemode()->getTable()->InInfluenceRange(Influence->getX(), Influence->getY(), Influence->InfluenceRange, PlaceTile->getX(), PlaceTile->getY(), BuildingSize))
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
						return PlaceTile->CanBuildOnTile(CurrentBuilding.bDeleteRescources);
					}
				}
			}

			if (PlaceTile)
			{
				int32 HalfX = (int32)BuildingSize.X / 2;
				int32 HalfY = (int32)BuildingSize.Y / 2;
				
				for (int32 x = -HalfX; x <= HalfX; x++)
				{
					for (int32 y = -HalfY; y <= HalfY; y++)
					{
						UTileData* Tile = getGamemode()->getTile(PlaceTile->getX() + x, PlaceTile->getY() + y);
						if (Tile)
						{
							if (CurrentBuilding.BlockedTiles.Contains(Tile->getTileType()))
							{
								Tile->DebugHighlightTile(0.0f, FColor::Red);
								return false;
							}

							if (!Tile->CanBuildOnTile(CurrentBuilding.bDeleteRescources))
							{
								Tile->DebugHighlightTile(0.0f, FColor::Red);
								return false;
							}

							Tile->DebugHighlightTile(0.0f, FColor::Green);
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

float ATablePlayerPawn::getZoomAlpha()
{
	return ZoomAlpha;
}

ATableGamemode* ATablePlayerPawn::getGamemode()
{
	if(!GM)
	{
		GM = Cast<ATableGamemode>(UGameplayStatics::GetGameMode(this));
	}

	return GM;
}

UTableGameInstance* ATablePlayerPawn::getGameInstance()
{
	if(!GI)
	{
		GI = Cast<UTableGameInstance>(GetGameInstance());
	}

	return GI;
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

FVector2D ATablePlayerPawn::getBuildingSize(bool bisRotated)
{
	if(bisRotated)
	{
		return FVector2D(CurrentBuilding.BuildingSize.Y, CurrentBuilding.BuildingSize.X);
	}

	return CurrentBuilding.BuildingSize;
}

FVector ATablePlayerPawn::getBuildingBuildLocation(UTileData* Tile)
{
	if (Tile)
	{
		float X = Tile->getX() * 100.0f + 50.0f;
		float Y = Tile->getY() * 100.0f + 50.0f;

		return FVector(X, Y, 0.0f);
	}

	return FVector::ZeroVector;
}

FVector2D ATablePlayerPawn::getBuildingBuildLocationTile(UTileData* Tile)
{
	if(Tile)
	{
		int32 X = Tile->getX() - 1;
		int32 Y = Tile->getY() - 1;

		return FVector2D(X, Y);
	}

	return FVector2D::ZeroVector;
}

