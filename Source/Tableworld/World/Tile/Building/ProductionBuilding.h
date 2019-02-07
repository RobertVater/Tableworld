// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "World/Tile/Building/BuildableTile.h"
#include "InventoryTile.h"
#include "ProductionBuilding.generated.h"

class ABaseCreature;
class ACityCentreTile;

UCLASS()
class TABLEWORLD_API AProductionBuilding : public AInventoryTile
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	TArray<FProductionItem> InputItemsData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	TArray<FProductionItem> OutputItemsData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	TMap<EItem, int32> InputItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	EItem OutputItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	int32 OutputItemAmount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	float RescourceCheckTime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	float ProductionTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	int32 MaxHaulers = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	TSubclassOf<AHaulerCreature> HaulerClass = nullptr;

	AProductionBuilding();
	virtual void Place(FVector PlaceLoc, TArray<FVector2D> nPlacedOnTiles, FTableBuilding nBuildingData, bool bRotated, bool bLoadBuilding) override;
	virtual void StartWork() override;
	virtual void StopWork() override;

	virtual void TryProduceOutput();

	void SpawnHaulers();
	AHaulerCreature* SpawnHauler(FVector Location);

	void ModifyInventory(EItem Item, int32 Amount, TMap<EItem, int32>& Storage);

	//Checks if the given inventory is valid and contains the input items we need
	virtual bool CheckIfValidInventory(ACityCentreTile* InInventory, UTileData*& StartTile, UTileData*& EndTile);

	//Hauler
	UFUNCTION()
	virtual void SendHaulerToInventory(ACityCentreTile* InInventory, UTileData* StartTile, UTileData* EndTile);

	UFUNCTION()
	void OnHaulerReachTarget(AHaulerCreature* Hauler);

	UFUNCTION()
	void OnHaulerReturnHome(AHaulerCreature* Hauler);

	virtual void ProduceItems();
	virtual void OnItemProduce();

	virtual EItem getItemType() override;
	virtual TArray<FProductionItem> getInputItems() override;
	virtual TArray<FProductionItem> getOutputItems() override;

	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "Getter")
	float getProductionProgress();

	virtual void LoadData(FTableSaveProductionBuilding Data);
	virtual void SaveData_Implementation(UTableSavegame* Savegame) override;

protected:

	float LoadProgress = 0.0f;

	ACityCentreTile* LastValidInventory = nullptr;

	TArray<AHaulerCreature*> Haulers;

	TMap<EItem, int32> InputStorage;
	TMap<EItem, int32> OutputStorage;

	FTimerHandle RescourceCheckTimer;
	FTimerHandle ProductionTimer;
};
