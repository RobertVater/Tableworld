// Fill out your copyright notice in the Description page of Project Settings.

#include "WaterTile.h"

ETileType UWaterTile::getTileType()
{
	return ETileType::Water;
}

float UWaterTile::getBaseHeigth()
{
	return -1.0f;
}

int32 UWaterTile::getMovementCost()
{
	return 50;
}