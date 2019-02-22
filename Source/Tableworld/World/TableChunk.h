// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Misc/TableHelper.h"
#include "ProceduralMeshComponent.h"
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

	void CleanupMemory();

	virtual void SetupChunk(uint8 nX, uint8 nY, ATableWorldTable* nParentTable);

	virtual void GenerateTileData();
	virtual void GenerateChunkMesh();

	//Mesh
	virtual void AddPlane(float X, float Y, int32 VertIndex, TArray<FVector>& Verts, TArray<FVector2D>& UVs);

	//Cube
	virtual void AddCubeFace(int32 X, int32 Y, int32 Z, ETableDirections Dir, int32& i);
	virtual void AddCube(int32 X, int32 Y, int32 Z, int32& i);

	virtual void AddTriangle(int32 a, int32 b, int32 c);

	virtual void UpdateChunkTexture();

	UTileData* SetTile(int32 X, int32 Y, ETileType NewTileType, bool bUpdateMaterial = true, bool bModifyTile = false);
	void SetTileIfTile(int32 X, int32 Y, ETileType NewTile, ETileType IfTile, bool bUpdateMaterial = true);

	void SetTileTexture(int32 X, int32 Y, ETileType Type);

	void UpdateTileHeights();
	void InsertPlane(int32 X, int32 Y, float Height);
	void ChangeTileHeight(int32 X, int32 Y, float Heigth);
	void ChangeVertHeight(int32 WorldX, int32 WorldY, float Heigth);
	void RebuildMesh();

	int32 getX();
	int32 getY();

	UTileData* getTile(int32 X, int32 Y);
	UTileData* getLocalTile(int32 X, int32 Y);

protected:

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FProcMeshTangent> Tangents;
	TArray<FVector2D> UVs;

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
