// Copyright by Robert Vater (Gunschlinger)

#include "BuildingInfoPanel.h"
#include "World/Tile/Building/BuildableTile.h"

void UBuildingInfoPanel::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if(SelectedBuilding)
	{
		APlayerController* PC = GetOwningPlayer();
		if (PC)
		{
			FVector2D ScreenLoc;
			PC->ProjectWorldLocationToScreen(SelectedBuilding->getWorldCenter(), ScreenLoc);

			SetPositionInViewport(ScreenLoc);
		}
	}
}

void UBuildingInfoPanel::OpenBuildingInfo(ABuildableTile* nSelectedBuilding)
{
	if(!nSelectedBuilding)
	{
		SetVisibility(ESlateVisibility::Hidden);
		SelectedBuilding = nullptr;

		return;
	}
	
	SelectedBuilding = nSelectedBuilding;

	//Set the location of the widget to the buildings world location
	if(SelectedBuilding)
	{
		//Setup the default data
		SetupInfo(SelectedBuilding);

		//Initial move
		APlayerController* PC = GetOwningPlayer();
		if (PC)
		{
			FVector2D ScreenLoc;
			PC->ProjectWorldLocationToScreen(SelectedBuilding->getWorldCenter(), ScreenLoc);

			SetPositionInViewport(ScreenLoc);
		}
		
		SetVisibility(ESlateVisibility::Visible);
	}
}

void UBuildingInfoPanel::SetupInfo_Implementation(ABuildableTile* Building)
{

}

ABuildableTile* UBuildingInfoPanel::getSelectedBuilding()
{
	return SelectedBuilding;
}
