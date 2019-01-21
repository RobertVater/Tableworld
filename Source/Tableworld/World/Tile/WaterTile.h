// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "World/Tile/TileData.h"
#include "WaterTile.generated.h"

/**
 * 
 */
UCLASS()
class TABLEWORLD_API UWaterTile : public UTileData
{
	GENERATED_BODY()
	
public:

	virtual ETileType getTileType() override;
	virtual float getBaseHeigth() override;
	virtual int32 getMovementCost() override;
	virtual FColor getMinimapColor() override;
};
