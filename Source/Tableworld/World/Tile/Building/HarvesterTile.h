// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "BuildableTile.h"
#include "HarvesterTile.generated.h"

class AHarvesterCreature;

UCLASS()
class TABLEWORLD_API AHarvesterTile : public ABuildableTile
{
	GENERATED_BODY()
	
public:

	//The Rescource this harvester will harvest
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvester")
	ETileRescources HarvestRescource = ETileRescources::None;

	//The item this harvester produces
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvester")
	EItem ProducedItems;

	//The time it takes for a Harvester to harvest 1 rescource
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvester")
	float HarvestTime = 1.0f;

	//The Inventory size of the harvester
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvester")
	int32 InventorySize = 5;

	//The searcharea of the upper left tile
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvester")
	int32 SearchRangeInTiles = 5;

	//The maximum number of creatures that can work for this building
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvester")
	int32 MaxHarvesterCreatureCount = 1;

	//The class of the harvester
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvester")
	TSubclassOf<AHarvesterCreature> WorkerClass = nullptr;

	virtual void Place(TArray<FVector2D> nPlacedOnTiles, FTableBuilding nBuildingData) override;
	virtual void StartWork() override;

	virtual void TryCreateCreature();
	virtual AHarvesterCreature* SpawnCreature();

	virtual void OnWorkerReturn(AHarvesterCreature* Worker);

	//Iterates through all harvestable tiles and removes tiles that have no rescources left
	void UpdateHarvestableTiles();
	UTileData* getNextHarvestTile();

	virtual int32 getBuildGridRadius() override;

protected:

	//A Array containing all HarvesterCreatures that got created by this actor
	TArray<AHarvesterCreature*> Workers;

	//A Array containing all tiles this harvester can harvest
	TArray<UTileData*> HarvestAbleTiles;

	//How many items this harvester has stored
	int32 CurrentInventory = 0;
};
