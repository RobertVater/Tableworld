// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "TableHelper.h"
#include "TableCheats.generated.h"

/**
 * 
 */
UCLASS()
class TABLEWORLD_API UTableCheats : public UCheatManager
{
	GENERATED_BODY()
	
public:

	UFUNCTION(Exec)
	void Give(EItem ItemID = EItem::WoodLog, int32 Amount = 1);

	UFUNCTION(Exec)
	void Starter();

	UFUNCTION(Exec)
	void SpawnFloater();
};
