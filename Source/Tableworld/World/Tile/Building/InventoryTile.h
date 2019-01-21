// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "World/Tile/Building/BuildableTile.h"
#include "InventoryTile.generated.h"

class AHaulerCreature;

UCLASS()
class TABLEWORLD_API AInventoryTile : public ABuildableTile
{
	GENERATED_BODY()
	
public:

	//The Inventory size of the building
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 InventorySize = 5;

	virtual void TransferInventory(AHaulerCreature* Hauler);

	virtual EItem getItemType();

	virtual bool canBeHauled();
	int32 getStoredItemCount();

	virtual int32 getCurrentStorage() override;
	virtual int32 getMaxStorage() override;


protected:

	//How many items this building has stored
	int32 CurrentInventory = 0;
};
