// Fill out your copyright notice in the Description page of Project Settings.

#include "HudManager.h"
#include "Player/TablePlayerController.h"
#include "UI/MainHud.h"
#include "Tableworld.h"
#include "UI/BuildingInfoPanel.h"

UHudManager::UHudManager()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UHudManager::BeginPlay()
{
	Super::BeginPlay();
}

void UHudManager::BuildUI()
{
	DebugLog("BuildUI");

	if(MainHud_Ref)
	{
		MainHud_Ref->RemoveFromParent();
		MainHud_Ref = nullptr;
	}

	if(BuildingInfoPanel_Ref)
	{
		BuildingInfoPanel_Ref->RemoveFromParent();
		BuildingInfoPanel_Ref = nullptr;
	}
	
	//MainHud
	if(Mainhud_Class)
	{
		MainHud_Ref = CreateWidget<UMainHud>(getPlayerController(), Mainhud_Class);
		if(MainHud_Ref)
		{
			MainHud_Ref->AddToPlayerScreen(0);
		}
	}

	//BuildingInfoPanel
	if(BuildingInfoPanel_Class)
	{
		BuildingInfoPanel_Ref = CreateWidget<UBuildingInfoPanel>(getPlayerController(), BuildingInfoPanel_Class);
		if(BuildingInfoPanel_Ref)
		{
			BuildingInfoPanel_Ref->AddToPlayerScreen(0);
			BuildingInfoPanel_Ref->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UHudManager::ResetUI()
{
	if(getMainHud())
	{
		getMainHud()->NativeConstruct();
	}
}

void UHudManager::ShowBuildingInfoPanel(ABuildableTile* SelectedBuilding)
{
	if(getBuildingInfoPanel())
	{
		getBuildingInfoPanel()->OpenBuildingInfo(SelectedBuilding);
	}
}

ATablePlayerController* UHudManager::getPlayerController()
{
	if(!PC)
	{
		PC = Cast<ATablePlayerController>(GetWorld()->GetFirstPlayerController());
	}

	return PC;
}

UMainHud* UHudManager::getMainHud()
{
	return MainHud_Ref;
}

UBuildingInfoPanel* UHudManager::getBuildingInfoPanel()
{
	return BuildingInfoPanel_Ref;
}
