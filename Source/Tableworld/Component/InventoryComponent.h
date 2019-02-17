// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TABLEWORLD_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 MaxInventory = 5;

	UInventoryComponent();

	void ModifyInventory(EItem Item, int32 Amount);
	void ClearInventory();

	bool AddReservedItem(TMap<EItem, int32> Items, FName UID);
	void ClearReservedItem(FName UID);

	bool HasInventorySpace();

	TMap<EItem, int32> getInventory();

	bool getItemZero(EItem& OutItem, int32& OutAmount);
	bool getItemAmount(EItem Item, int32& OutAmount);
	bool hasItem(EItem Item);
	bool hasItems(TMap<EItem, int32> Items, bool bIgnoreReserved = true);
	bool hasEnoughItems(EItem Item, int32 Amount);

	//Gets the amount of all items in the inventory
	int32 getInventorySize();

protected:

	TArray<FReservedItem> ReservedItems;
	TMap<EItem, int32> Inventory;
};
