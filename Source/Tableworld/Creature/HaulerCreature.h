// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "Creature/BaseCreature.h"
#include "Misc/TableHelper.h"
#include "HaulerCreature.generated.h"

class AInventoryTile;
class ACityCentreTile;
class AProductionBuilding;
class UTileData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHaulerReachedTarget, AHaulerCreature*, Hauler);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHaulerReturnedHome, AHaulerCreature*, Hauler);

UCLASS()
class TABLEWORLD_API AHaulerCreature : public ABaseCreature
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	UAnimationAsset* WalkLoaded = nullptr;

	//Events
	UPROPERTY()
	FHaulerReachedTarget Event_HaulerReachedTarget;

	UPROPERTY()
	FHaulerReturnedHome Event_HaulerReturnedHome;

	void GiveHaulJob(AInventoryTile* nTargetInventory, ACityCentreTile* nHomeCityCentre, UTileData* nTargetTile, UTileData* nHomeTile);
	void GiveProductionHaulJob(ACityCentreTile* nInventory, AProductionBuilding* nHomeProductionBuilding, UTileData* nTargetTile, UTileData* nHomeTile);

	void GiveReturnJob();
	void ForceReturnJob();

	void AddHaulItems(EItem nItem, int32 nAmount);

	virtual void OnMoveCompleted() override;

	//Get the first item of the carried list.
	bool getItemZero(EItem& Item, int32& Amount);
	TMap<EItem, int32> getCarriedItems();

	bool HasItems();

	ACityCentreTile* getInventoryBuilding();
	AProductionBuilding* getHomeProductionBuilding();

	virtual UAnimationAsset* getWalkAnimation() override;

protected:

	UTileData* HaulTile = nullptr;

	//Inventory
	AInventoryTile* TargetInventory = nullptr;
	ACityCentreTile* HomeCityCentre = nullptr;

	//ProductionBuildings
	ACityCentreTile* Inventory = nullptr;
	AProductionBuilding* HomeProductionBuilding = nullptr;

	bool bHauledItems = false;
	TMap<EItem, int32> CarriedItems;
};
