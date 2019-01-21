// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Tableworld.h"
#include "GameFramework/Actor.h"
#include "Misc/TableHelper.h"
#include "TableWorldTable.generated.h"

class UTileData;
class ATableChunk;
class UFastNoise;
class UInstancedStaticMeshComponent;
class ATablePlayerController;

class ABuildableTile;
class ACityCentreTile;

struct FRescourceWobble
{
	ETileRescources Rescource = ETileRescources::None;
	int32 InstanceIndex = 0;

	float WobbleRenewLife = 0.25f;

	float MaxLife = 1.0f;
	float Life = 1.0f;
};

UCLASS()
class TABLEWORLD_API ATableWorldTable : public AActor
{
	GENERATED_BODY()
	
public:	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	USceneComponent* Root = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	UStaticMesh* TileMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	int32 TilesInPixels = 32;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	UMaterial* ChunkMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	UTexture2D* TileSheet = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	TMap<ETileType, FVector2D> TileMaterials;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	TMap<ETileRescources, UStaticMesh*> RescourceMesh;

	//The max width in chunks of the map
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGeneration")
	int32 MaxSizeX = 8;

	//The max depth in chunks of the map
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGeneration")
	int32 MaxSizeY = 8;

	//The size of a chunk in tiles
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGeneration")
	int32 ChunkSize = 16;

	//The size of the tiles.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGeneration")
	int32 TileSize = 100;

	//True if a river should be generated
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGeneration")
	bool bHasRiver = true;

	//The size in tiles of the river
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGeneration")
	uint8 RiverSize = 2;

	ATableWorldTable();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void AddRescourceWobble(ETileRescources Rescource, int32 Index, float Life);

	//Generated a new map
	virtual void GenerateMap();
	virtual void GenerateChunks();

	virtual UTexture2D* GenerateMinimap();

	UFUNCTION(meta = (BlueprintThreadSafe))
	virtual void UpdateMinimap(TArray<UTileData*> ModifiedTiles);

	//Breaks down the tile sheet and stores the pixels into a array
	virtual void SetupTilePixels(TArray<ETileType> TileTypes);

	ATableChunk* getChunkForTile(int32 X, int32 Y);
	UTileData* getTile(int32 X, int32 Y);
	TArray<UTileData*> getTilesInRadius(int32 X, int32 Y, int32 Radius);
	TArray<UTileData*> getRescourcesInRadius(int32 X, int32 Y, int32 Radius, ETileRescources Rescource);

	bool HarvestRescource(UTileData* Tile, int32 Amount);
	void SetRescource(int32 X, int32 Y, ETileRescources Res, int32 Amount, ETileType NeededType);
	void SetTile(int32 X, int32 Y, ETileType type, bool bUpdateTexture = false);
	void SetTileIfTile(int32 X, int32 Y, ETileType NewTile, ETileType IfTile);

	void AddBuilding(ABuildableTile* nBuilding);

	TArray<FColor> getTilePixels(ETileType TileType);

	ATablePlayerController* getPlayerController();
	UFastNoise* getNoise();

	//Pathfinding

	/*
	Finds a path from the StartTile to the EndTile
	Arg0 = The Start of the search
	Arg1 = The End of the search
	Arg2 = A Array of forbidden tiles that will act like they are blocked tiles
	Arg3 = True if the path can go diagonal
	Arg4 = True if we should consider the Movement Penalty of tiles
	*/
	TArray<UTileData*> FindPath(FVector2D StartTile, FVector2D EndTile, TArray<ETileType> ForbidenTiles = TArray<ETileType>(), bool bAllowDiag = false, bool bIgnoreWeigths = false, TArray<ETileType> AllowedTiles = TArray<ETileType>());
	TArray<UTileData*> RetracePath(UTileData* Start, UTileData* End);
	TArray<UTileData*> GetNeighbours(UTileData* Tile, bool bAllowDiag);

	//Road Pathfinding
	TArray<UTileData*> FindPathRoad(UTileData* StartTile, UTileData* EndTile, bool bAllowDiag = false);

	int32 getTileDistance(UTileData* TileA, UTileData* TileB);
	int32 getDistance(int32 X, int32 Y, int32 EX, int32 EY);

	TArray<ABuildableTile*> getBuildings();
	TArray<ACityCentreTile*> getCityCentres();

	UTexture2D* getMinimapTexture();

protected:

	ATablePlayerController* PC = nullptr;

	TArray<FRescourceWobble> RescourceWobble;

	UPROPERTY()
	UTexture2D* MinimapTexture = nullptr;

	UPROPERTY()
	TArray<ABuildableTile*> Buildings;

	UPROPERTY()
	TMap<ETileRescources, UInstancedStaticMeshComponent*> InstancedRescourcesMesh;

	UPROPERTY()
	UFastNoise* Noise = nullptr;

	UPROPERTY()
	TArray<ATableChunk*> Chunks;

	UPROPERTY()
	TArray<FTilePixels> TilePixels;
};
