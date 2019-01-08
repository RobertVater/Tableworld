// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildableTile.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

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
	virtual void Place(int32 X, int32 Y);

protected:

	UMaterialInterface* DefaultMaterial = nullptr;

	bool bIsGhost = false;

	int32 TileX = 0;
	int32 TileY = 0;
};
