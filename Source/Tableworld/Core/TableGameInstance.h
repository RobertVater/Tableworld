// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Misc/TableHelper.h"
#include "TableGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class TABLEWORLD_API UTableGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable,BlueprintNativeEvent,Category = "Data")
	FTableBuilding getBuilding(const FName& ID);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Data")
	TArray<FTableBuilding> getBuildingsForAgeAndCategory(ETableAge Age, ETableTileCategory Category);

};
