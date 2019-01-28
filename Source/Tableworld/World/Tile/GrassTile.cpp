// Fill out your copyright notice in the Description page of Project Settings.

#include "GrassTile.h"

ETileType UGrassTile::getTileType()
{
	return ETileType::Grass;
}

int32 UGrassTile::getMovementCost()
{
	return 10;
}

FColor UGrassTile::getMinimapColor()
{
	if(HasRescource() || HasTileObject())
	{
		return Super::getMinimapColor();
	}
	
	return FColor::FromHex("#263617");
}
