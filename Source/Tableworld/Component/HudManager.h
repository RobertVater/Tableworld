// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HudManager.generated.h"

class ATablePlayerController;
class UMainHud;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TABLEWORLD_API UHudManager : public UActorComponent
{
	GENERATED_BODY()

public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UMainHud> Mainhud_Class = nullptr;

	UHudManager();
	virtual void BeginPlay() override;

	virtual void BuildUI();

	ATablePlayerController* getPlayerController();

	UMainHud* getMainHud();

protected:

	ATablePlayerController* PC = nullptr;

	UMainHud* MainHud_Ref = nullptr;
};
