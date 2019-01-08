// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TablePlayerController.generated.h"

class UHudManager;

UCLASS()
class TABLEWORLD_API ATablePlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	ATablePlayerController();
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable,BlueprintNativeEvent, Category = "Getter")
	UHudManager* getHudManager();
};
