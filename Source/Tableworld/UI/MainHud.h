// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainHud.generated.h"

/**
 * 
 */
UCLASS()
class TABLEWORLD_API UMainHud : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable,BlueprintNativeEvent,Category = "Building")
	void BuildBuildableTiles();
};
