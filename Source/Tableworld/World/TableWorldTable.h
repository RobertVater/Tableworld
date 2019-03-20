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
class UHierarchicalInstancedStaticMeshComponent;
class ATablePlayerController;
class ATableGamemode;
class UTableGameInstance;

class ABuildableTile;
class ACityCentreTile;
class UInfluenceComponent;
class Thread_Pathfinder;

struct FRescourceWobble
{
	ATableChunk* ParentChunk = nullptr;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UInstancedStaticMeshComponent* WaterPlanes = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
	FTableGrass GrassSettings;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	UStaticMesh* GrassMesh = nullptr;

	ATableWorldTable();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void AddRescourceWobble(ETileRescources Rescource, int32 Index, float Life, ATableChunk* nParentChunk);

	//Generated a new map
	virtual void InitTable(int32 Seed, uint8 WorldSize, bool bHasRiver, uint8 RiverCount);
	virtual void GenerateMap();
	virtual void GenerateChunks();

	//Minimap
	void ClearMinimap();
	virtual UTexture2D* GenerateMinimap();

	UFUNCTION(meta = (BlueprintThreadSafe))
	virtual void UpdateMinimap(TArray<UTileData*> ModifiedTiles);

	//Breaks down the tile sheet and stores the pixels into a array
	virtual void SetupTilePixels(TArray<ETileType> TileTypes);

	void SetMultipleTiles(TArray<UTileData*> Tiles);
	ATableChunk* getChunkForTile(int32 X, int32 Y);
	UTileData* getTile(int32 X, int32 Y);
	TArray<UTileData*> getTilesInRadius(int32 X, int32 Y, int32 Radius, ETileType Tile, bool bBlockRescources);
	TArray<UTileData*> getRescourcesInRadius(int32 X, int32 Y, int32 Radius, TArray<ETileRescources> Rescource);
	int32 getNumTilesAroundTile(int32 X, int32 Y, ETileType Type);
	virtual bool InInfluenceRange(int32 CenterX, int32 CenterY, int32 Radius, int32 X, int32 Y, FVector2D Size);

	bool HarvestRescource(UTileData* Tile, int32 Amount);
	void SetRescource(int32 X, int32 Y, ETileRescources Res, ETileType NeededType);
	UTileData* SetTile(int32 X, int32 Y, ETileType type, bool bUpdateTexture = false, bool bModifyTile = false);
	void SetTileIfTile(int32 X, int32 Y, ETileType NewTile, ETileType IfTile);
	void LoadTile(FGeneratedMapTile Data);

	void AddBuilding(ABuildableTile* nBuilding);
	void RemoveBuilding(ABuildableTile* nBuilding);

	//Grid
	void ShowInfluenceGrid();
	void HideInfluenceGrid();

	TArray<FColor> getTilePixels(ETileType TileType);

	ATablePlayerController* getPlayerController();
	ATableGamemode* getGamemode();
	UTableGameInstance* getGameInstance();
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
	TArray<UTileData*> FindPath(FTablePathfindingRequest Request);
	TArray<UTileData*> RetracePath(UTileData* Start, UTileData* End);
	TArray<UTileData*> GetNeighbours(UTileData* Tile, bool bAllowDiag);

	//Road Pathfinding
	TArray<UTileData*> FindPathRoad(UTileData* StartTile, UTileData* EndTile, bool bAllowDiag = false);

	ABuildableTile* getBuildingWithID(FName UID);
	TArray<ABuildableTile*> getBuildings();
	TArray<ACityCentreTile*> getCityCentres();
	TArray<UInfluenceComponent*> getInfluenceComponents();

	ACityCentreTile* getNearestCityCenter(int32 X, int32 Y);

	UTexture2D* getMinimapTexture();

	int32 getNewRandomSeed();
	int32 getRandomSeed();

	uint8 getWorldSize();

	//Interface
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interface")
	void SaveData(UTableSavegame* Savegame);
	virtual void SaveData_Implementation(UTableSavegame* Savegame);

protected:

	Thread_Pathfinder* Pathfinder = nullptr;

	int32 Seed = 0;
	uint8 WorldSize = 0;
	bool bHasRiver = false;
	uint8 RiverCount = 0;

	ATablePlayerController* PC = nullptr;
	ATableGamemode* GM = nullptr;
	UTableGameInstance* GI = nullptr;

	TArray<FRescourceWobble> RescourceWobble;

	UPROPERTY()
	UTexture2D* MinimapTexture = nullptr;

	UPROPERTY()
	TArray<ABuildableTile*> Buildings;

	UPROPERTY()
	UFastNoise* Noise = nullptr;

	UPROPERTY()
	TArray<ATableChunk*> Chunks;

	UPROPERTY()
	TArray<FTilePixels> TilePixels;
};
