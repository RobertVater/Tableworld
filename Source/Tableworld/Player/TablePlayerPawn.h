// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Misc/TableHelper.h"
#include "TablePlayerPawn.generated.h"

class UTileData;
class ATableGamemode;
class ABuildableTile;
class ATablePlayerController;
class USpringArmComponent;
class UCameraComponent;

UCLASS()
class TABLEWORLD_API ATablePlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Root = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArm = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* Camera = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom")
	float MinZoom = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom")
	float MaxZoom = 2000.0f;

	ATablePlayerPawn();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void SetCurrentBuilding(FTableBuilding Building);

	virtual void Input_LeftMouse_Pressed();
	virtual void Input_LeftMouse_Released();

	virtual void Input_MiddleMouse_Pressed();
	virtual void Input_MiddleMouse_Released();

	virtual void Input_RightMouse_Pressed();
	virtual void Input_RightMouse_Released();

	virtual void Input_ZoomIn();
	virtual void Input_ZoomOut();
	
	virtual void Input_Forward(float v);
	virtual void Input_Right(float v);

	virtual void PlaceBuilding(int32 X, int32 Y,  FTableBuilding BuildingData);

	virtual void AdjustZoom();

	bool HasValidBuildableTile();
	bool CanBuild(FVector2D BuildingSize, UTileData* PlaceTile);

	ATableGamemode* getGamemode();
	ATablePlayerController* getPlayerController();

	FVector getMouseWorldLocation();
	FVector getMouseWorldLocationGrid();

	FVector getBuildingBuildLocation(UTileData* Tile);

protected:

	//Building
	UTileData* SelectedTile = nullptr;

	FTableBuilding CurrentBuilding;

	ABuildableTile* TileActor = nullptr;

	bool bIsDragBuilding = false;

	int32 MaxDragTiles = 32;

	FVector2D DragTileLocation;
	FVector2D StartDragTile;
	FVector2D EndDragTile;
	TArray<FVector2D> DragTiles;

	ATableGamemode* GM = nullptr;
	ATablePlayerController* PC = nullptr;

	float ZoomAlpha = 0.0f;

	//Camera Rotation
	FVector2D CamRotStart,CamRotDif,CamRotEnd;
	bool bRotateCamera = false;

};
