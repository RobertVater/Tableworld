// Fill out your copyright notice in the Description page of Project Settings.

#include "WaterTile.h"

ETileType UWaterTile::getTileType()
{
	return ETileType::Water;
}

float UWaterTile::getBaseHeigth()
{
	return -50.0f;
}

int32 UWaterTile::getMovementCost()
{
	return 50;
}

FColor UWaterTile::getMinimapColor()
{
	if (HasRescource() || HasTileObject())
	{
		return Super::getMinimapColor();
	}

	return FColor::FromHex("#0077be");
}
