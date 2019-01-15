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
