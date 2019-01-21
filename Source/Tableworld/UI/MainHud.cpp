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

	ATableGamemode* GM = Cast<ATableGamemode>(UGameplayStatics::GetGameMode(this));
	if(GM)
	{
		GM->Event_StoredItemsUpdated.AddDynamic(this, &UMainHud::UpdateRescourceUI);
		UpdateRescourceUI();
	}
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
