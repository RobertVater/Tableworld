// Fill out your copyright notice in the Description page of Project Settings.

#include "SandTile.h"

ETileType USandTile::getTileType()
{
	return ETileType::Sand;
}

int32 USandTile::getMovementCost()
{
	return 15;
}

FColor USandTile::getMinimapColor()
{
	if (HasRescource())
	{
		return Super::getMinimapColor();
	}

	return FColor::Yellow;
}
