// Copyright by Robert Vater (Gunschlinger)

#include "DirtRoadTile.h"

ETileType UDirtRoadTile::getTileType()
{
	return ETileType::DirtRoad;
}

int32 UDirtRoadTile::getMovementCost()
{
	return 0;
}

FColor UDirtRoadTile::getMinimapColor()
{
	return FColor::Red;
}
