// Copyright by Robert Vater (Gunschlinger)

#include "ProductionBuilding.h"
#include "Creature/HaulerCreature.h"

AProductionBuilding::AProductionBuilding()
{
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
}

void AProductionBuilding::TransferInventory(AHaulerCreature* Hauler)
{
	if (Hauler)
	{
		for (auto Elem : InventoryComponent->getInventory()) 
		{
			Hauler->AddHaulItems(Elem.Key, Elem.Value);
			DebugError("HArvester " + FString::FromInt((int32)Elem.Key) + " x" + FString::FromInt(Elem.Value));
		}

		//Clear our inventory and set us free from being haul locked
		InventoryComponent->ClearInventory();

		SetHaulLocked(false);
	}
}

void AProductionBuilding::ModifyInventory(EItem Item, int32 Amount)
{
	InventoryComponent->ModifyInventory(Item, Amount);
}

EItem AProductionBuilding::getItemType()
{
	return EItem::None;
}

bool AProductionBuilding::HasInventorySpace()
{
	return (getCurrentStorage() < getMaxStorage());
}

bool AProductionBuilding::canBeHauled()
{
	float v = (float)getStoredItemCount() / (float)getMaxStorage();

	return (v >= getHaulTreshold());
}

int32 AProductionBuilding::getStoredItemCount()
{
	return InventoryComponent->getInventorySize();
}

TArray<ETileRescources> AProductionBuilding::getOutputChoices()
{
	return TArray<ETileRescources>();
}

TArray<ETileRescources> AProductionBuilding::getInputChoices()
{
	return TArray<ETileRescources>();
}

int32 AProductionBuilding::getCurrentStorage()
{
	return getStoredItemCount();
}

int32 AProductionBuilding::getMaxStorage()
{
	return InventoryComponent->MaxInventory;
}

FTableInfoPanel AProductionBuilding::getInfoPanelData_Implementation()
{
	FTableInfoPanel Data = Super::getInfoPanelData_Implementation();

	Data.InventoryComponent = InventoryComponent;
	
	return Data;
}
