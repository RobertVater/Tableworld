// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "Misc/TableHelper.h"
#include "WorkerCreature.h"
#include "HaulerCreature.generated.h"

class AProductionBuilding;
class ACityCentreTile;
class AFactoryBuilding;
class UTileData;
class UInventoryComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHaulerReachedTarget, AHaulerCreature*, Hauler);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHaulerReturnedHome, AHaulerCreature*, Hauler);

UCLASS()
class TABLEWORLD_API AHaulerCreature : public AWorkerCreature
{
	GENERATED_BODY()
	
public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UInventoryComponent* InventoryComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	UAnimationAsset* WalkLoaded = nullptr;

	//Events
	UPROPERTY()
	FHaulerReachedTarget Event_HaulerReachedTarget;

	UPROPERTY()
	FHaulerReturnedHome Event_HaulerReturnedHome;

	AHaulerCreature();
	void GiveHaulJob(FName nTargetBuilding, FName nHomeBuilding, UTileData* nHomeTile, UTileData* nTargetTile);

	void GiveReturnJob();
	void ForceReturnJob();

	void AddHaulItems(EItem nItem, int32 nAmount);

	virtual void OnMoveCompleted() override;

	virtual void ClearInventory();

	TMap<EItem, int32> getCarriedItems();

	bool HasItems();

	FName getHomeBuildingUID();
	FName getTargetBuildingUID();

	virtual FName getReadableStatus() override;
	virtual UAnimationAsset* getWalkAnimation() override;

	virtual FTableInfoPanel getInfoPanelData_Implementation() override;

protected:

	UTileData* HaulTile = nullptr;

	FName HomeBuildingUID = "";
	FName TargetBuildingUID = "";

	bool bHauledItems = false;
};
