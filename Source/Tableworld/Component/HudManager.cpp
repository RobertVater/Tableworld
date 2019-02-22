// Fill out your copyright notice in the Description page of Project Settings.

#include "HudManager.h"
#include "Player/TablePlayerController.h"
#include "UI/MainHud.h"
#include "Tableworld.h"
#include "UI/InfoPanel.h"

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

	if(InfoPanel_Ref)
	{
		InfoPanel_Ref->RemoveFromParent();
		InfoPanel_Ref = nullptr;
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

	//InfoPanel
	if(InfoPanel_Class)
	{
		InfoPanel_Ref = CreateWidget<UInfoPanel>(getPlayerController(), InfoPanel_Class);
		if(InfoPanel_Ref)
		{
			InfoPanel_Ref->AddToPlayerScreen(0);
			InfoPanel_Ref->SetVisibility(ESlateVisibility::Hidden);
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

void UHudManager::ShowBuildingInfoPanel(FTableInfoPanel PanelData)
{
	if(getInfoPanel())
	{
		getInfoPanel()->OpenPanel(PanelData);
	}
}

void UHudManager::OnHumanDeactivated(AActor* Human)
{
	
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

UInfoPanel* UHudManager::getInfoPanel()
{
	return InfoPanel_Ref;
}