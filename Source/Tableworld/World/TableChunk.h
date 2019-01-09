// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Misc/TableHelper.h"
#include "TableChunk.generated.h"

class UProceduralMeshComponent;
class UTileData;
class ATableWorldTable;

UCLASS()
class TABLEWORLD_API ATableChunk : public AActor
{
	GENERATED_BODY()
	
public:	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UProceduralMeshComponent* ChunkMesh = nullptr;

	ATableChunk();
	virtual void BeginPlay() override;

	virtual void SetupChunk(uint8 nX, uint8 nY, ATableWorldTable* nParentTable);

	virtual void GenerateTileData();
	virtual void GenerateChunkMesh();

	virtual void UpdateChunkTexture();

	void SetTile(int32 X, int32 Y, ETileType NewTileType, bool bUpdateMaterial = true);
	void SetTileIfTile(int32 X, int32 Y, ETileType NewTile, ETileType IfTile, bool bUpdateMaterial = true);

	void SetTileTexture(int32 X, int32 Y, ETileType Type);

	void ChangeTileHeight(int32 X, int32 Y, float Heigth);
	void ChangeVertHeight(int32 WorldX, int32 WorldY, float Heigth);
	void UpdateMesh();

	int32 getX();
	int32 getY();

	UTileData* getTile(int32 X, int32 Y);

protected:

	TArray<FVector> Vertices;


	uint8 TileSize = 100;

	uint8 X = 0;
	uint8 Y = 0;
	int32 ChunkSize = 0;

	UPROPERTY()
	TArray<UTileData*> Tiles;

	UPROPERTY()
	ATableWorldTable* ParentTable = nullptr;

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial = nullptr;

	UPROPERTY()
	UTexture2D* ChunkTexture = nullptr;
};
