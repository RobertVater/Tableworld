// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HudManager.generated.h"

class ATablePlayerController;
class ABuildableTile;

class UMainHud;
class UBuildingInfoPanel;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TABLEWORLD_API UHudManager : public UActorComponent
{
	GENERATED_BODY()

public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UMainHud> Mainhud_Class = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UBuildingInfoPanel> BuildingInfoPanel_Class = nullptr;

	UHudManager();
	virtual void BeginPlay() override;

	virtual void BuildUI();
	virtual void ResetUI();

	void ShowBuildingInfoPanel(ABuildableTile* SelectedBuilding);

	ATablePlayerController* getPlayerController();

	UMainHud* getMainHud();
	UBuildingInfoPanel* getBuildingInfoPanel();

protected:

	ATablePlayerController* PC = nullptr;

	UMainHud* MainHud_Ref = nullptr;
	UBuildingInfoPanel* BuildingInfoPanel_Ref = nullptr;
};
