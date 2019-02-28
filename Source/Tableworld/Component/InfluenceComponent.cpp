// Copyright by Robert Vater (Gunschlinger)

#include "InfluenceComponent.h"
#include "World/Tile/Building/BuildableTile.h"

UInfluenceComponent::UInfluenceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInfluenceComponent::ShowInfluenceRadius()
{
	if(getParentBuilding())
	{
		getParentBuilding()->BuildGrid(InfluenceRange);
	}
}

void UInfluenceComponent::HideInfluenceRadius()
{
	if (getParentBuilding())
	{
		getParentBuilding()->ClearGridRadius();
	}
}

ABuildableTile* UInfluenceComponent::getParentBuilding()
{
	if(!ParentBuilding)
	{
		ParentBuilding = Cast<ABuildableTile>(GetOwner());
	}

	return ParentBuilding;
}

int32 UInfluenceComponent::getX()
{
	if(getParentBuilding())
	{
		return getParentBuilding()->getCenterX();
	}

	return 0;
}

int32 UInfluenceComponent::getY()
{
	if (getParentBuilding())
	{
		return getParentBuilding()->getCenterY();
	}

	return 0;
}
