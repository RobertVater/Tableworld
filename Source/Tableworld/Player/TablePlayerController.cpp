// Fill out your copyright notice in the Description page of Project Settings.

#include "TablePlayerController.h"
#include "Component/HudManager.h"

ATablePlayerController::ATablePlayerController()
{

}

void ATablePlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;

	if(getHudManager())
	{
		getHudManager()->BuildUI();
	}
}

UHudManager* ATablePlayerController::getHudManager_Implementation()
{
	return nullptr;
}
