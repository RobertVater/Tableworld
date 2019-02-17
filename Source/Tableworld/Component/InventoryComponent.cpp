// Copyright by Robert Vater (Gunschlinger)

#include "InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::ModifyInventory(EItem Item, int32 Amount)
{
	if (Item == EItem::None && Item == EItem::Max)return;

	if (!Inventory.Contains(Item))
	{
		if (Amount > 0)
		{
			Inventory.Add(Item, Amount);
		}

		return;
	}

	int32 OldAmount = Inventory.FindRef(Item) + Amount;

	if (OldAmount <= 0)
	{
		Inventory.Remove(Item);
		return;
	}

	Inventory.Emplace(Item, OldAmount);
}

void UInventoryComponent::ClearInventory()
{
	Inventory.Empty();
}

bool UInventoryComponent::AddReservedItem(TMap<EItem, int32> Items, FName UID)
{
	if (hasItems(Items))
	{
		//Reserve the items
		for (auto Elem : Items)
		{
			EItem Item = Elem.Key;
			int32 Amount = Elem.Value;

			FReservedItem ReservedItem;
			ReservedItem.Item = Item;
			ReservedItem.Amount = Amount;
			ReservedItem.BuildingUID = UID;

			ReservedItems.Add(ReservedItem);
		}

		return true;
	}

	return false;
}

void UInventoryComponent::ClearReservedItem(FName UID)
{
	if (UID != NAME_None)
	{
		for (int32 i = 0; i < ReservedItems.Num(); i++)
		{
			FReservedItem Item = ReservedItems[i];
			if (Item.BuildingUID.IsEqual(UID))
			{
				ReservedItems.RemoveAt(i);
			}
		}
	}
}

bool UInventoryComponent::HasInventorySpace()
{
	return (getInventory().Num() < MaxInventory);
}

TMap<EItem, int32> UInventoryComponent::getInventory()
{
	return Inventory;
}

bool UInventoryComponent::getItemZero(EItem& OutItem, int32& OutAmount)
{
	for(auto Elem : Inventory)
	{
		OutItem = Elem.Key;
		OutAmount = Elem.Value;
		return true;
	}
	
	return false;
}

bool UInventoryComponent::getItemAmount(EItem Item, int32& OutAmount)
{
	for (auto Elem : Inventory)
	{
		if (Elem.Key == Item)
		{
			OutAmount = Elem.Value;
			return true;
		}
	}

	return false;
}

bool UInventoryComponent::hasItem(EItem Item)
{
	for (auto Elem : Inventory)
	{
		if (Elem.Key == Item)
		{
			return true;
		}
	}

	return false;
}

bool UInventoryComponent::hasItems(TMap<EItem, int32> Items, bool bIgnoreReserved /*= true*/)
{
	TMap<EItem, int32> Inventory = getInventory();
	if (!bIgnoreReserved)
	{
		//Subtract the reserved items from the inventory
		for (int32 i = 0; i < ReservedItems.Num(); i++)
		{
			FReservedItem Reserve = ReservedItems[i];
			if (Inventory.Contains(Reserve.Item))
			{
				int32 Amount = Inventory.FindRef(Reserve.Item);
				if (Reserve.Amount >= Amount)
				{
					Inventory.Remove(Reserve.Item);
				}
				else
				{
					int32 NewAmount = Amount - Reserve.Amount;
					Inventory.Emplace(Reserve.Item, NewAmount);
				}
			}
		}
	}

	for (auto Elem : Items)
	{
		EItem Item = Elem.Key;
		int32 Amount = Elem.Value;

		if (Inventory.Contains(Item))
		{
			int32 FoundAmount = getInventory().FindRef(Item);
			if (FoundAmount < Amount)
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool UInventoryComponent::hasEnoughItems(EItem Item, int32 Amount)
{
	for (auto Elem : Inventory)
	{
		if (Elem.Key == Item)
		{
			if (Elem.Value >= Amount) 
			{
				return true;
			}

			return false;
		}
	}
	
	return false;
}

int32 UInventoryComponent::getInventorySize()
{
	int32 Amount = 0;

	for(auto Elem : Inventory)
	{
		Amount += Elem.Value;
	}

	return Amount;
}
