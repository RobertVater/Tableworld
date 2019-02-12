// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "World/Tile/TileData.h"
#include "DirtRoadTile.generated.h"

/**
 * 
 */
UCLASS()
class TABLEWORLD_API UDirtRoadTile : public UTileData
{
	GENERATED_BODY()
	
public:

	virtual ETileType getTileType() override;
	virtual int32 getMovementCost() override;
};
