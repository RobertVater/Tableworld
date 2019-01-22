// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "InventoryTile.h"
#include "HarvesterTile.generated.h"

class AHarvesterCreature;

UENUM(BlueprintType)
enum class EHarvesterType : uint8
{
	Rescource,
	Tile
};

UCLASS()
class TABLEWORLD_API AHarvesterTile : public AInventoryTile
{
	GENERATED_BODY()
	
public:

	//The type of object this harvester is harvesting
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvester")
	EHarvesterType HarvesterType = EHarvesterType::Rescource;

	//The Rescource this harvester will harvest
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvester")
	ETileRescources HarvestRescource = ETileRescources::None;

	//What tile to harvest
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvester")
	ETileType HarvestTile = ETileType::Rock;

	//The item this harvester produces
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvester")
	EItem ProducedItems;

	//The time it takes for a Harvester to harvest 1 rescource
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvester")
	float HarvestTime = 1.0f;

	//The searcharea of the upper left tile
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvester")
	int32 SearchRangeInTiles = 5;

	//The maximum number of creatures that can work for this building
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvester")
	int32 MaxWorkerCount = 1;

	//The class of the harvester
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvester")
	TSubclassOf<AHarvesterCreature> WorkerClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvester")
	TArray<FProductionItem> InputItemsData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvester")
	TArray<FProductionItem> OutputItemsData;

	//Called when we place down the building
	virtual void Place(TArray<FVector2D> nPlacedOnTiles, FTableBuilding nBuildingData) override;

	//Resumes work for this building
	virtual void StartWork() override;

	//Pauses work for this building
	virtual void StopWork() override;

	//Try to spawn only so many worker as our storage allows
	virtual void InitWorkers();

	virtual AHarvesterCreature* SpawnWorker();
	virtual void DeactivateWorker(AHarvesterCreature* Worker);
	virtual void ActivateWorker(AHarvesterCreature* Worker);

	//Increases the stored item count by 1
	virtual bool StoreItem();

	//Called once a hauler has arrived at this building
	virtual void TransferInventory(AHaulerCreature* Hauler) override;

	//Called once a worker has returned from work (To drop off rescources etc)
	virtual void OnWorkerReturn(AHarvesterCreature* Worker);

	//Iterates through all harvestable tiles and removes tiles that have no rescources left
	void UpdateHarvestableTiles();

	//Get the nearest valid Harvest tile
	UTileData* getNextHarvestTile();

	bool HasInventorySpace();
	virtual int32 getBuildGridRadius() override;
	virtual EItem getItemType() override;

	virtual TArray<FProductionItem> getInputItems() override;
	virtual TArray<FProductionItem> getOutputItems() override;

protected:

	//A Array containing all HarvesterCreatures that got created by this actor
	TArray<AHarvesterCreature*> Workers;

	//A Array containing all tiles this harvester can harvest
	TArray<UTileData*> HarvestAbleTiles;
};
