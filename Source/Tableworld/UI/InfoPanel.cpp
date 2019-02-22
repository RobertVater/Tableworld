// Copyright by Robert Vater (Gunschlinger)

#include "InfoPanel.h"

void UInfoPanel::OpenPanel(FTableInfoPanel nPanelData)
{
	if (!nPanelData.WorldContext)
	{
		SetVisibility(ESlateVisibility::Hidden);
		TargetActor = nullptr;
		PanelData = FTableInfoPanel();

		DebugWarning("Hide");
		return;
	}

	TargetActor = nPanelData.WorldContext;
	PanelData = nPanelData;

	bHasStaticLocation = false;
	if(nPanelData.StaticWorldLocation != FVector::ZeroVector)
	{
		bHasStaticLocation = true;
		StaticWorldLocation = nPanelData.StaticWorldLocation;
	}

	//Set the location of the widget to the buildings world location
	if (TargetActor)
	{
		//Setup the default data
		SetupInfo(PanelData);

		//Initial move
		APlayerController* PC = GetOwningPlayer();
		if (PC)
		{
			FVector2D ScreenLoc;
			PC->ProjectWorldLocationToScreen(getTargetWorldLocation(), ScreenLoc);

			SetPositionInViewport(ScreenLoc);
		}

		SetVisibility(ESlateVisibility::Visible);
	}
}

void UInfoPanel::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (TargetActor)
	{
		APlayerController* PC = GetOwningPlayer();
		if (PC)
		{
			FVector2D ScreenLoc;
			PC->ProjectWorldLocationToScreen(getTargetWorldLocation(), ScreenLoc);

			SetPositionInViewport(ScreenLoc);
		}
	}
}

void UInfoPanel::SetupInfo_Implementation(FTableInfoPanel PanelData)
{

}

AActor* UInfoPanel::getWorldContextActor()
{
	return TargetActor;
}

FVector UInfoPanel::getTargetWorldLocation()
{
	if(bHasStaticLocation)
	{
		return StaticWorldLocation;
	}
	
	if (getWorldContextActor()) 
	{
		return getWorldContextActor()->GetActorLocation();
	}
	
	return FVector::ZeroVector;
}

FTableInfoPanel UInfoPanel::getPanelData()
{
	return PanelData;
}
