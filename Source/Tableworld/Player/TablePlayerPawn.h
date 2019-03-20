// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Misc/TableHelper.h"
#include "Savegame/TableSavegame.h"
#include "TablePlayerPawn.generated.h"

class UTileData;
class ATableGamemode;
class UTableGameInstance;
class ABuildableTile;
class ATablePlayerController;
class USpringArmComponent;
class UCameraComponent;
class UBillboardComponent;

class UInstancedStaticMeshComponent;

class UMaterialExpressionCollectionParameter;

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
	UBillboardComponent* CameraGoal = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* Camera = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInstancedStaticMeshComponent* DragBuildingMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInstancedStaticMeshComponent* InstancedHighlightMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* HighlightMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	USoundBase* BuildingPlaceSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	USoundBase* BuildingRemoveSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	UParticleSystem* BuildingPlaceParticles = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	UParticleSystem* BuildingRemoveParticles = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom")
	UMaterialParameterCollection* GlobalMaterialVariables = nullptr;;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom")
	float ZoomSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom")
	float MinZoom = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom")
	float MaxZoom = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom")
	float MinPitch = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom")
	float MaxPitch = 89.0f;

	ATablePlayerPawn();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void UpdateCamera(float Delta);
	void MoveSelectedBuilding();

	//FX
	void PlaySound2D(USoundBase* Sound);

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

	virtual void Input_Rotate();

	//Time
	virtual void Input_GameSpeed_0();
	virtual void Input_GameSpeed_1();
	virtual void Input_GameSpeed_2();
	virtual void Input_GameSpeed_3();
	virtual void Input_GameSpeed_4();

	UFUNCTION(BlueprintCallable, Category = "Time")
	virtual void ModifyGameSpeed(int32 SpeedLevel);

	virtual void TryPlaceBuilding(int32 X, int32 Y,  FTableBuilding BuildingData);
	virtual ABuildableTile* PlaceBuilding(UTileData* Tile, float Rotation, FTableBuilding BuildingData);

	UFUNCTION(BlueprintCallable, Category = "Destruction")
	virtual void ActivateDestroyMode(bool bDestroyMode);

	virtual int32 AddHighlight(FVector Location);
	virtual void RemoveHighlight(int32 Index);
	virtual void ClearHighlight();

	virtual bool DestroyBuilding(UTileData* Tile);

	virtual void AdjustZoom();

	UFUNCTION(BlueprintCallable,Category = "Teleport")
	virtual void TeleportPlayer(FVector NewLocation);

	virtual void SetZoomAlpha(float nZoomAlpha);

	virtual void UpdateMinimapPlayerView();

	bool HasValidBuildableTile();
	bool CanBuild(FVector2D BuildingSize, UTileData* PlaceTile);

	float getZoomAlpha();
	ATableGamemode* getGamemode();
	UTableGameInstance* getGameInstance();
	ATablePlayerController* getPlayerController();

	FVector2D getScreenMouseLocation();
	FVector getMouseWorldLocation();
	FVector getMouseWorldLocationGrid();

	FVector2D getBuildingSize(bool bisRotated);
	FVector getBuildingBuildLocation(UTileData* Tile);
	FVector2D getBuildingBuildLocationTile(UTileData* Tile);

protected:

	UPROPERTY()
	UAudioComponent* UISound = nullptr;

	ABuildableTile* LastHighlightBuilding = nullptr;
	bool bDestructionmode = false;

	//Building
	UTileData* SelectedTile = nullptr;
	bool bRotated = false;

	FTableBuilding CurrentBuilding;

	ABuildableTile* BuildingGhost = nullptr;

	bool bIsDragBuilding = false;

	int32 MaxDragTiles = 32;

	FVector2D DragTileLocation;
	FVector2D StartDragTile;
	FVector2D EndDragTile;
	TArray<UTileData*> DragTiles;

	ATableGamemode* GM = nullptr;
	UTableGameInstance* GI = nullptr;
	ATablePlayerController* PC = nullptr;

	float ZoomLerpGoal = 1.0f;
	float ZoomAlpha = 1.0f;

	//Camera Rotation
	FVector2D CamRotStart,CamRotDif,CamRotEnd;
	bool bRotateCamera = false;

	//Camera Drag
	bool bDraggingCamera = false;
	FVector2D CameraDragStart;
	FVector2D CameraDragDif;
	FVector2D CameraDragEnd;

};
