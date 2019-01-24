// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Misc/TableHelper.h"
#include "BuildableTile.generated.h"

class UInstancedStaticMeshComponent;
class UStaticMeshComponent;
class UBoxComponent;

class ATableWorldTable;
class ATableGamemode;
class UTableGameInstance;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UInstancedStaticMeshComponent* GridHighlight = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	UMaterial* GhostMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	EBuildingInfoType InfoPanelType = EBuildingInfoType::Production;

	ABuildableTile();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void ShowGridRadius();
	virtual void ClearGridRadius();

	virtual void SetIsGhost(FTableBuilding nBuildingData);
	virtual void Place(TArray<FVector2D> nPlacedOnTiles, FTableBuilding nBuildingData);

	virtual void SetIsBlocked(bool bBlocked);
	virtual void SetHaulLocked(bool bHaulLocked);

	UFUNCTION(BlueprintCallable, Category = "Building")
	virtual void StartWork();

	UFUNCTION(BlueprintCallable, Category = "Building")
	virtual void StopWork();

	//Returns the nearest unblocked tile around this building
	UTileData* getValidTile();
	UTileData* getTile();

	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "Getter")
	ATableGamemode* getGamemode();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	UTableGameInstance* getGameInstance();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	ATableWorldTable* getTable();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	bool isWorking();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	int32 getTileX();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	int32 getTileY();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	FVector2D getBuildingSize();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	FVector getWorldCenter();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	FTableBuilding getBuildingData();


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	virtual int32 getBuildGridRadius();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	virtual int32 getCurrentStorage();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	virtual int32 getMaxStorage();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	virtual float getHaulTreshold();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	virtual TArray<FProductionItem> getInputItems();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	virtual TArray<FProductionItem> getOutputItems();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	virtual FColor getMinimapColor();

	UFUNCTION()
	TArray<UTileData*> getTilesAroundUs(bool bForceRegenerate = false);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	bool isHaulerComming();

protected:

	bool bHaulerIsComming = false;

	UTableGameInstance* GI = nullptr;
	ATableGamemode* GM = nullptr;
	ATableWorldTable* ParentTable = nullptr;

	//A array with the global tile locations that this building is placed on!
	TArray<FVector2D> PlacedOnTiles;

	//The tiles around this building
	UPROPERTY()
	TArray<UTileData*> TilesAroundUs;

	UMaterialInstanceDynamic* DynMaterial = nullptr;
	UMaterialInterface* DefaultMaterial = nullptr;

	FTableBuilding BuildingData;

	bool bIsWorking = false;
	bool bIsGhost = false;

	int32 TileX = 0;
	int32 TileY = 0;
};
