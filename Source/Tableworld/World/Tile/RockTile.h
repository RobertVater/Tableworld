// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "World/Tile/TileData.h"
#include "RockTile.generated.h"

/**
 * 
 */
UCLASS()
class TABLEWORLD_API URockTile : public UTileData
{
	GENERATED_BODY()
	
public:

	virtual ETileType getTileType() override;
	virtual int32 getMovementCost() override;
	virtual float getBaseHeigth() override;
};
