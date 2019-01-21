// Fill out your copyright notice in the Description page of Project Settings.

#include "RockTile.h"

ETileType URockTile::getTileType()
{
	return ETileType::Rock;
}

int32 URockTile::getMovementCost()
{
	return 40;
}

FColor URockTile::getMinimapColor()
{
	if (HasRescource())
	{
		return Super::getMinimapColor();
	}

	return FColor(100, 100, 100);
}
