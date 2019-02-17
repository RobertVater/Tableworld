// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "World/Tile/Building/BuildableTile.h"
#include "ProductionBuilding.generated.h"

class AHaulerCreature;

UCLASS()
class TABLEWORLD_API AProductionBuilding : public ABuildableTile
{
	GENERATED_BODY()
	
public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UInventoryComponent* InventoryComponent = nullptr;

	AProductionBuilding();
	virtual void TransferInventory(AHaulerCreature* Hauler);
	virtual void ModifyInventory(EItem Item, int32 Amount);

	virtual EItem getItemType();

	bool HasInventorySpace();
	virtual bool canBeHauled();
	int32 getStoredItemCount();

	virtual int32 getCurrentStorage() override;
	virtual int32 getMaxStorage() override;
};
