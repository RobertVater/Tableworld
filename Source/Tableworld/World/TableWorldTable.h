// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Tableworld.h"
#include "GameFramework/Actor.h"
#include "Misc/TableHelper.h"
#include "Interface/SaveLoadInterface.h"
#include "Savegame/TableSavegame.h"
#include "MapGenerator.h"
#include "TableWorldTable.generated.h"

class UTileData;
class ATableChunk;
class UFastNoise;
class UInstancedStaticMeshComponent;
class ATablePlayerController;
class ATableGamemode;

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
class TABLEWORLD_API ATableWorldTable : public AActor, public ISaveLoadInterface
{
	GENERATED_BODY()
	
public:	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	USceneComponent* Root = nullptr;

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

	ATableWorldTable();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void AddRescourceWobble(ETileRescources Rescource, int32 Index, float Life);

	//Generated a new map
	virtual void InitTable(int32 Seed, uint8 WorldSize, bool bHasRiver, uint8 RiverCount);
	virtual void GenerateMap();
	virtual void GenerateChunks();

	virtual UTexture2D* GenerateMinimap();

	UFUNCTION(meta = (BlueprintThreadSafe))
	virtual void UpdateMinimap(TArray<UTileData*> ModifiedTiles);

	//Breaks down the tile sheet and stores the pixels into a array
	virtual void SetupTilePixels(TArray<ETileType> TileTypes);

	void SetMultipleTiles(TArray<UTileData*> Tiles);
	ATableChunk* getChunkForTile(int32 X, int32 Y);
	UTileData* getTile(int32 X, int32 Y);
	TArray<UTileData*> getTilesInRadius(int32 X, int32 Y, int32 Radius, ETileType Tile, bool bBlockRescources);
	TArray<UTileData*> getRescourcesInRadius(int32 X, int32 Y, int32 Radius, ETileRescources Rescource);
	virtual bool InInfluenceRange(int32 CenterX, int32 CenterY, int32 Radius, int32 X, int32 Y, FVector2D Size);

	bool HarvestRescource(UTileData* Tile, int32 Amount);
	void SetRescource(int32 X, int32 Y, ETileRescources Res, int32 Amount, ETileType NeededType);
	void SetTile(int32 X, int32 Y, ETileType type, bool bUpdateTexture = false, bool bModifyTile = false);
	void SetTileIfTile(int32 X, int32 Y, ETileType NewTile, ETileType IfTile);
	void LoadTile(FGeneratedMapTile Data);

	void AddBuilding(ABuildableTile* nBuilding);
	void ShowInfluenceGrid();
	void HideInfluenceGrid();

	TArray<FColor> getTilePixels(ETileType TileType);

	FTransform getRescourceTransform(ETileRescources Rescource, int32 Index);
	ATablePlayerController* getPlayerController();
	ATableGamemode* getGamemode();
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

	ABuildableTile* getBuildingWithID(FName UID);
	TArray<ABuildableTile*> getBuildings();
	TArray<ACityCentreTile*> getCityCentres();

	UTexture2D* getMinimapTexture();

	int32 getNewRandomSeed();
	int32 getRandomSeed();

	uint8 getWorldSize();

	virtual void LoadData(TArray<FTableSaveTile> Tiles);

	//Interface
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interface")
	void SaveData(UTableSavegame* Savegame);
	virtual void SaveData_Implementation(UTableSavegame* Savegame);

protected:

	int32 Seed = 0;
	uint8 WorldSize = 0;
	bool bHasRiver = false;
	uint8 RiverCount = 0;

	ATablePlayerController* PC = nullptr;
	ATableGamemode* GM = nullptr;

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
