// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "World/Tile/TileData.h"
#include "DirtTile.generated.h"

/**
 * 
 */
UCLASS()
class TABLEWORLD_API UDirtTile : public UTileData
{
	GENERATED_BODY()
	
public:

	virtual ETileType getTileType() override;
	virtual int32 getMovementCost() override;
};
