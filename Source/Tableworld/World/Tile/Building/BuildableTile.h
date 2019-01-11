// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Misc/TableHelper.h"
#include "BuildableTile.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

class ATableWorldTable;
class ATableGamemode;

UCLASS()
class TABLEWORLD_API ABuildableTile : public AActor
{
	GENERATED_BODY()
	
public:	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	USceneComponent* Root = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UStaticMeshComponent* TileMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UBoxComponent* CollisionBox = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	UMaterial* GhostMaterial = nullptr;

	ABuildableTile();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void SetIsGhost();
	virtual void Place(TArray<FVector2D> nPlacedOnTiles, FTableBuilding nBuildingData);

	virtual void SetIsBlocked(bool bBlocked);

	virtual void StartWork();
	virtual void StopWork();

	//Returns the nearest unblocked tile around this building
	UTileData* getValidTile();
	UTileData* getTile();

	ATableGamemode* getGamemode();
	ATableWorldTable* getTable();

	int32 getTileX();
	int32 getTileY();

	TArray<UTileData*> getTilesAroundUs();

protected:

	ATableGamemode* GM = nullptr;
	ATableWorldTable* ParentTable = nullptr;

	//A array with the global tile locations that this building is placed on!
	TArray<FVector2D> PlacedOnTiles;

	//The tiles around this building
	TArray<UTileData*> TilesAroundUs;

	UMaterialInstanceDynamic* DynMaterial = nullptr;
	UMaterialInterface* DefaultMaterial = nullptr;

	FTableBuilding BuildingData;

	bool bIsWorking = false;
	bool bIsGhost = false;

	int32 TileX = 0;
	int32 TileY = 0;
};
