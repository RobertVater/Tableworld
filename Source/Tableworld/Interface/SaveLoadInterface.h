// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Interface.h"
#include "SaveLoadInterface.generated.h"

class UTableSavegame;

UINTERFACE()
class TABLEWORLD_API USaveLoadInterface : public UInterface
{
	GENERATED_BODY()
};

class ISaveLoadInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Save")
	void SaveData(UTableSavegame* Savegame);
};
