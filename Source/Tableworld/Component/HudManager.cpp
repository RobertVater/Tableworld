// Fill out your copyright notice in the Description page of Project Settings.

#include "HudManager.h"
#include "Player/TablePlayerController.h"
#include "UI/MainHud.h"
#include "Tableworld.h"

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
	DebugWarning("BuildUI");
	
	if(Mainhud_Class)
	{
		MainHud_Ref = CreateWidget<UMainHud>(getPlayerController(), Mainhud_Class);
		if(MainHud_Ref)
		{
			MainHud_Ref->AddToPlayerScreen(0);
		}
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
