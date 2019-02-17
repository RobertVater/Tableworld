// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "World/Tile/Building/BuildableTile.h"
#include "CityCentreBuilding.generated.h"

class UWorkerComponent;
class UInventoryComponent;

class AWorkerCreature;
class AProductionBuilding;
class UTableSavegame;

UCLASS()
class TABLEWORLD_API ACityCentreTile : public ABuildableTile
{
	GENERATED_BODY()
	
public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UWorkerComponent* WorkerComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UInventoryComponent* InventoryComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CityCentre")
	int32 InfluenceRadius = 10;

	//The delay whenever the building checks for harvesters with inventory
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CityCentre")
	float RescourceCheckTime = 1.0f;

	ACityCentreTile();
	virtual void Place(FVector PlaceLoc, TArray<FVector2D> nPlacedOnTiles, FTableBuilding nBuildingData, bool bNewRotated, bool bLoadBuilding) override;
	virtual void StartWork() override;
	virtual void StopWork() override;

	virtual int32 getBuildGridRadius() override;

	void OnRescourceCheck();

	UFUNCTION()
	void OnHaulerReturnHome(AHaulerCreature* nHauler);

	UFUNCTION()
	void OnHaulReachedTarget(AHaulerCreature* nHauler);

	void ModifyInventory(EItem Item, int32 Amount);

	//Reserves the items to a building. Those items are invisible to other buildings
	bool ReserveItems(TMap<EItem, int32> Items, FName UID);
	void ClearReserveItems(FName UID);

	TArray<AWorkerCreature*> getWorkers();

	AProductionBuilding* getValidHaulGoal(FVector2D& InTile, FVector2D& OutTile);

	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "Getter")
	TMap<EItem, int32> getStoredItems();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	bool HasItems(TMap<EItem, int32> Items, bool bIgnoreReserved = false);

	virtual void SaveData_Implementation(UTableSavegame* Savegame) override;

protected:

	FTimerHandle RescourceCheckTimer;
};
