// Fill out your copyright notice in the Description page of Project Settings.

#include "MainHud.h"
#include "Core/TableGamemode.h"
#include "Kismet/GameplayStatics.h"

void UMainHud::BuildBuildableTiles_Implementation()
{

}

void UMainHud::UpdateRescourceUI_Implementation()
{

}

void UMainHud::NativeConstruct()
{
	Super::NativeConstruct();

	if(getGamemode())
	{
		getGamemode()->Event_StoredItemsUpdated.AddDynamic(this, &UMainHud::UpdateRescourceUI);
		getGamemode()->Event_GameSpeedUpdated.AddDynamic(this, &UMainHud::UpdateGameTime);
		getGamemode()->Event_NewNotification.AddDynamic(this, &UMainHud::OnNewNotification);

		UpdateRescourceUI();
		UpdateGameTime(1);
	}
}

void UMainHud::OnNewNotification_Implementation(FTableNotification NewNotify)
{

}

void UMainHud::MoveMinimapPlayerView_Implementation(float AlphaX, float AlphaY, float ZoomAlpha)
{

}

void UMainHud::UpdateMinimap_Implementation(UTexture2D* NewMinimap)
{

}

void UMainHud::ShowBuildMenu_Implementation(bool bShow)
{

}

void UMainHud::UpdateGameTime_Implementation(int32 NewGameTime)
{

}

void UMainHud::SelectTool(EToolbarTools NewTool)
{
	ShowBuildMenu(false);

	if(SelectedTool == NewTool || NewTool == EToolbarTools::None)
	{
		SelectedTool = EToolbarTools::None;
		return;
	}

	SelectedTool = NewTool;

	switch(SelectedTool)
	{
		case EToolbarTools::Build: ShowBuildMenu(true); break;
	}
}

EToolbarTools UMainHud::getSelectedTool()
{
	return SelectedTool;
}

ATableGamemode* UMainHud::getGamemode()
{
	return GM ? GM : GM = Cast<ATableGamemode>(UGameplayStatics::GetGameMode(GetOwningPlayer()));
}
