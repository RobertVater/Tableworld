// Copyright by Robert Vater (Gunschlinger)

#include "InventoryTile.h"
#include "Creature/HaulerCreature.h"

void AInventoryTile::TransferInventory(AHaulerCreature* Hauler)
{
	if(Hauler)
	{
		Hauler->AddHaulItems(getItemType(), getStoredItemCount());

		//Clear our inventory and set us free from being haul locked
		CurrentInventory = 0;
		SetHaulLocked(false);
	}
}

EItem AInventoryTile::getItemType()
{
	return EItem::None;
}

bool AInventoryTile::canBeHauled()
{
	float v = (float)getStoredItemCount() / (float)InventorySize;

	return (v >= 0.55f);
}

int32 AInventoryTile::getStoredItemCount()
{
	return CurrentInventory;
}
