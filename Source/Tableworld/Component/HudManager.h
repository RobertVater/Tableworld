// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Misc/TableHelper.h"
#include "HudManager.generated.h"

class ATablePlayerController;
class ABuildableTile;

class UMainHud;
class UInfoPanel;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TABLEWORLD_API UHudManager : public UActorComponent
{
	GENERATED_BODY()

public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UMainHud> Mainhud_Class = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UInfoPanel> InfoPanel_Class = nullptr;

	UHudManager();
	virtual void BeginPlay() override;

	virtual void BuildUI();
	virtual void ResetUI();

	void ShowBuildingInfoPanel(FTableInfoPanel PanelData);

	void OnHumanDeactivated(AActor* Human);

	ATablePlayerController* getPlayerController();

	UMainHud* getMainHud();
	UInfoPanel* getInfoPanel();

protected:

	ATablePlayerController* PC = nullptr;

	UMainHud* MainHud_Ref = nullptr;

	UInfoPanel* InfoPanel_Ref;
};
