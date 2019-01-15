// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "Creature/BaseCreature.h"
#include "Misc/TableHelper.h"
#include "HaulerCreature.generated.h"

class AInventoryTile;
class ACityCentreTile;
class UTileData;

UCLASS()
class TABLEWORLD_API AHaulerCreature : public ABaseCreature
{
	GENERATED_BODY()
	
public:

	void GiveHaulJob(AInventoryTile* nTargetInventory, ACityCentreTile* nHomeCityCentre, UTileData* nTargetTile, UTileData* nHomeTile);
	void GiveReturnJob();

	void AddHaulItems(EItem nItem, int32 nAmount);

	virtual void OnMoveCompleted() override;

	EItem getHaulItem();
	int32 getHaulAmount();

protected:

	UTileData* HaulTile = nullptr;
	AInventoryTile* TargetInventory = nullptr;
	ACityCentreTile* HomeCityCentre = nullptr;

	bool bHauledItems = false;
	int32 HaulAmount = 0;
	EItem HaulItem = EItem::None;
};
