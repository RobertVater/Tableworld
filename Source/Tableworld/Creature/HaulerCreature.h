// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "Creature/BaseCreature.h"
#include "Misc/TableHelper.h"
#include "HumanCreature.h"
#include "HaulerCreature.generated.h"

class AInventoryTile;
class ACityCentreTile;
class AProductionBuilding;
class UTileData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHaulerReachedTarget, AHaulerCreature*, Hauler);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHaulerReturnedHome, AHaulerCreature*, Hauler);

UCLASS()
class TABLEWORLD_API AHaulerCreature : public AHumanCreature
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

	void GiveHaulJob(FName nTargetBuilding, FName nHomeBuilding, UTileData* nTargetTile, UTileData* nHomeTile);

	void GiveReturnJob();
	void ForceReturnJob();

	void AddHaulItems(EItem nItem, int32 nAmount);

	virtual void OnMoveCompleted() override;

	virtual void ClearInventory();

	//Get the first item of the carried list.
	bool getItemZero(EItem& Item, int32& Amount);
	TMap<EItem, int32> getCarriedItems();

	bool HasItems();

	FName getHomeBuildingUID();
	FName getTargetBuildingUID();

	virtual UAnimationAsset* getWalkAnimation() override;

	void LoadData(FTableSaveHaulerCreature Data);
	FTableSaveHaulerCreature getSaveData();

protected:

	UTileData* HaulTile = nullptr;

	FName HomeBuildingUID = "";
	FName TargetBuildingUID = "";

	bool bHauledItems = false;
	TMap<EItem, int32> CarriedItems;
};
