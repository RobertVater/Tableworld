// Copyright by Robert Vater (Gunschlinger)

#include "InfoPanel.h"

void UInfoPanel::OpenPanel(FTableInfoPanel nPanelData)
{
	if (!nPanelData.WorldContext)
	{
		SetVisibility(ESlateVisibility::Hidden);
		TargetActor = nullptr;
		PanelData = FTableInfoPanel();

		return;
	}

	TargetActor = Cast<AActor>(nPanelData.WorldContext);
	PanelData = nPanelData;

	bHasStaticLocation = false;
	if(nPanelData.StaticWorldLocation != FVector::ZeroVector)
	{
		bHasStaticLocation = true;
		StaticWorldLocation = nPanelData.StaticWorldLocation;

		DebugLog("StaticLoc");
	}

	//Set the location of the widget to the buildings world location
	if (PanelData.bOpenPanel)
	{
		//Setup the default data
		SetupInfo(PanelData);

		//Initial move
		MoveWidgetInPosition();
		DebugLog("PanelSize " + PanelData.PanelSize.ToString());
		DebugLog("PanelSize HalfX " + FString::SanitizeFloat(PanelData.PanelSize.X / 2));

		SetVisibility(ESlateVisibility::Visible);
	}
}

void UInfoPanel::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	MoveWidgetInPosition();
}

void UInfoPanel::MoveWidgetInPosition()
{
	if (IsVisible()) 
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
