// Copyright by Robert Vater (Gunschlinger)

#include "CityCentreTile.h"

int32 ACityCentreTile::getBuildGridRadius()
{
	if (BuildingData.ID != NAME_None)
	{
		return InfluenceRadius + (int32)(BuildingData.BuildingSize.X + BuildingData.BuildingSize.Y);
	}

	return Super::getBuildGridRadius();
}
