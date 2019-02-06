// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "World/Tile/Building/BuildableTile.h"
#include "CityCentreTile.generated.h"

class AHaulerCreature;
class AInventoryTile;
class UTableSavegame;

UCLASS()
class TABLEWORLD_API ACityCentreTile : public ABuildableTile
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CityCentre")
	int32 InfluenceRadius = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CityCentre")
	int32 HaulerAmount = 2;

	//The delay whenever the building checks for harvesters with inventory
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CityCentre")
	float RescourceCheckTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CityCentre")
	TSubclassOf<AHaulerCreature> HaulerClass = nullptr;

	ACityCentreTile();
	virtual void Place(FVector PlaceLoc, TArray<FVector2D> nPlacedOnTiles, FTableBuilding nBuildingData, bool bNewRotated, bool bLoadBuilding) override;
	virtual void StartWork() override;
	virtual void StopWork() override;

	virtual int32 getBuildGridRadius() override;

	void OnRescourceCheck();

	UFUNCTION()
	void OnHaulCompleted(AHaulerCreature* nHauler);

	UFUNCTION()
	void OnHaulReachedTarget(AHaulerCreature* nHauler);

	UFUNCTION(meta = (BlueprintThreadSafe))
	void ModifyInventory(EItem Item, int32 Amount);

	//Reserves the items to a building. Those items are invisible to other buildings
	bool ReserveItems(TMap<EItem, int32> Items, FName UID);
	void ClearReserveItems(FName UID);

	AHaulerCreature* SpawnWorker(FVector SpawnLoc);

	AInventoryTile* getValidHaulGoal(FVector2D& InTile, FVector2D& OutTile);

	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "Getter")
	TMap<EItem, int32> getStoredItems();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	bool HasItems(TMap<EItem, int32> Items, bool bIgnoreReserved = false);

	virtual void LoadData(FTableSaveCityCenterBuilding Data);
	virtual void SaveData_Implementation(UTableSavegame* Savegame) override;

protected:

	TArray<AHaulerCreature*> Workers;

	//The items this city Centre has stored
	TMap<EItem, int32> StoredItems;

	TArray<FReservedItem> ReservedItems;

	FTimerHandle RescourceCheckTimer;
};
