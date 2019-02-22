// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Misc/TableHelper.h"
#include "InventoryComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TABLEWORLD_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 MaxInventory = 5;

	UInventoryComponent();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ModifyInventory(EItem Item, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ClearInventory();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddReservedItem(TMap<EItem, int32> Items, FName UID);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ClearReservedItem(FName UID);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	bool HasInventorySpace();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	TMap<EItem, int32> getInventory();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	bool getItemZero(EItem& OutItem, int32& OutAmount);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	bool getItemAmount(EItem Item, int32& OutAmount);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	bool hasItem(EItem Item);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	bool hasItems(TMap<EItem, int32> Items, bool bIgnoreReserved = true);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	bool hasEnoughItems(EItem Item, int32 Amount);

	//Gets the amount of all items in the inventory
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	int32 getInventorySize();

protected:

	TArray<FReservedItem> ReservedItems;
	TMap<EItem, int32> Inventory;
};
