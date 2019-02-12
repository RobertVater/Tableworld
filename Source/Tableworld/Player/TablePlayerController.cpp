// Fill out your copyright notice in the Description page of Project Settings.

#include "TablePlayerController.h"
#include "Component/HudManager.h"
#include "UI/MainHud.h"
#include "Core/TableGamemode.h"
#include "World/TableWorldTable.h"
#include "Kismet/GameplayStatics.h"

ATablePlayerController::ATablePlayerController()
{

}

void ATablePlayerController::BeginPlay()
{
	Super::BeginPlay();

	InitController();
}

void ATablePlayerController::InitController()
{
	bShowMouseCursor = true;

	if (getHudManager())
	{
		getHudManager()->BuildUI();
		bShouldPerformFullTickWhenPaused = true;
	}

	UpdateMinimap();
	ClientSetCameraFade(true, FColor::Black, FVector2D(1.0f, 0.0f), 2.5f, true);
}

void ATablePlayerController::SelectTool(EToolbarTools NewTool)
{
	if(getHudManager())
	{
		if (getHudManager()->getMainHud())
		{
			getHudManager()->getMainHud()->SelectTool(NewTool);
		}
	}
}

void ATablePlayerController::ShowBuildingInfoPanel(ABuildableTile* Building)
{
	if(getHudManager())
	{
		getHudManager()->ShowBuildingInfoPanel(Building);
	}
}

void ATablePlayerController::UpdateMinimap()
{
	if(getHudManager())
	{
		if(getHudManager()->getMainHud())
		{
			if(getTable())
			{
				getHudManager()->getMainHud()->UpdateMinimap(getTable()->getMinimapTexture());
			}
		}
	}
}

void ATablePlayerController::UpdateMinimapPlayerView(float AlphaX, float AlphaY, float ZoomAlpha, float ViewScale)
{
	if (getHudManager())
	{
		if (getHudManager()->getMainHud())
		{
			if (getTable())
			{
				getHudManager()->getMainHud()->MoveMinimapPlayerView(AlphaX, AlphaY, ZoomAlpha, ViewScale);
			}
		}
	}
}

ATableGamemode* ATablePlayerController::getGamemode()
{
	if(!GM)
	{
		GM = Cast<ATableGamemode>(UGameplayStatics::GetGameMode(this));
	}

	return GM;
}

ATableWorldTable* ATablePlayerController::getTable()
{
	if(!Table)
	{
		if(getGamemode())
		{
			Table = getGamemode()->getTable();
		}
	}

	return Table;
}

UHudManager* ATablePlayerController::getHudManager_Implementation()
{
	return nullptr;
}
