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
