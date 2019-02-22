// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "World/Tile/Building/BuildableTile.h"
#include "ProductionBuilding.generated.h"

class AHaulerCreature;
class UInventoryComponent;

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

	virtual bool HasInventorySpace();
	virtual bool canBeHauled();
	virtual int32 getStoredItemCount();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	virtual TArray<ETileRescources> getOutputChoices();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	virtual TArray<ETileRescources> getInputChoices();

	virtual int32 getCurrentStorage() override;
	virtual int32 getMaxStorage() override;

	virtual FTableInfoPanel getInfoPanelData_Implementation() override;
};
