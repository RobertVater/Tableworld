// Copyright by Robert Vater (Gunschlinger)

#include "DirtTile.h"

ETileType UDirtTile::getTileType()
{
	return ETileType::Dirt;
}

int32 UDirtTile::getMovementCost()
{
	return 5;
}

FColor UDirtTile::getMinimapColor()
{
	return FColor::Orange;
}
