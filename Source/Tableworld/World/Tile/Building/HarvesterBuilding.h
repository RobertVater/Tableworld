// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "ProductionBuilding.h"
#include "HarvesterBuilding.generated.h"

class AHarvesterCreature;
class UWorkerComponent;

UENUM(BlueprintType)
enum class EHarvesterType : uint8
{
	Rescource,
	Tile
};

UCLASS()
class TABLEWORLD_API AHarvesterTile : public AProductionBuilding
{
	GENERATED_BODY()
	
public:

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Components")
	UWorkerComponent* WorkerComponent = nullptr;

	//The type of object this harvester is harvesting
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvester")
	EHarvesterType HarvesterType = EHarvesterType::Rescource;

	//The Rescource this harvester will harvest
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvester")
	TArray<ETileRescources> HarvestRescources;

	//What tile to harvest
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvester")
	ETileType HarvestTile = ETileType::Rock;

	//The item this harvester produces
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvester")
	TArray<EItem> ProducedItems;

	//The time it takes for a Harvester to harvest 1 rescource
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvester")
	float HarvestTime = 1.0f;

	//The time is takes for this building to output the item. 0 if instant
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvester")
	float ProduceTime = 0.0f;

	//The searcharea of the upper left tile
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvester")
	int32 SearchRangeInTiles = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvester")
	TArray<FProductionItem> InputItemsData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvester")
	TArray<FProductionItem> OutputItemsData;

	AHarvesterTile();

	//Called when we place down the building
	virtual void Place(FVector PlaceLoc, TArray<FVector2D> nPlacedOnTiles, FTableBuilding nBuildingData, bool bNewRotated, bool bLoadBuilding) override;

	//Resumes work for this building
	virtual void StartWork() override;

	//Pauses work for this building
	virtual void StopWork() override;

	//Try to spawn only so many worker as our storage allows
	virtual void InitWorkers();

	virtual bool GiveWork(AHarvesterCreature* Worker, bool bForceWork = false);

	//Notifications
	void Notification_FullStorage();
	void Notification_NoRescources();

	//Adds a Itemtoken to be processed later
	virtual void AddRawItem();

	//Called once the ProduceItem reached 0. Will use up a token and produce a full item
	virtual void OnItemProduction();

	//Increases the stored item count by 1
	virtual bool StoreItem(EItem Item);

	//Called once a hauler has arrived at this building
	virtual void TransferInventory(AHaulerCreature* Hauler) override;

	//Called once a worker has returned from work (To drop off rescources etc)
	virtual void OnWorkerReturn(AHarvesterCreature* Worker);

	UFUNCTION(BlueprintCallable,Category = "Harvesterbuilding")
	virtual void OnInputItemChanged(int32 Index);

	virtual AHarvesterCreature* getDeactivatedWorker();
	virtual void TrySendWorkerToWork(AHarvesterCreature* Worker);

	//Iterates through all harvestable tiles and removes tiles that have no rescources left
	void UpdateHarvestableTiles();

	//Get the nearest valid Harvest tile
	UTileData* getNextHarvestTile(ETileRescources Rescource = ETileRescources::None);

	virtual int32 getBuildGridRadius() override;
	virtual FColor getGridColor() override;
	virtual float getGridHeigth() override;
	virtual EItem getItemType() override;

	virtual EItem getProducedItem();
	virtual ETileRescources getHarvestRescource();

	TArray<AWorkerCreature*> getWorkers();

	virtual TArray<FProductionItem> getInputItems() override;
	virtual TArray<FProductionItem> getOutputItems() override;

	virtual TArray<ETileRescources> getInputChoices() override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	virtual int32 getHarvestRescourceIndex();

	virtual void SaveData_Implementation(UTableSavegame* Savegame) override;
	
	virtual FTableInfoPanel getInfoPanelData_Implementation() override;

protected:

	FTimerHandle ItemProductionTimer;
	int32 RawItems = 0;

	int32 HarvestRescourceIndex = 0;

	//A Array containing all tiles this harvester can harvest
	TArray<UTileData*> HarvestAbleTiles;
};
