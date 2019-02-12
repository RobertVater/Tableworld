// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "World/Tile/TileData.h"
#include "GrassTile.generated.h"

/**
 * 
 */
UCLASS()
class TABLEWORLD_API UGrassTile : public UTileData
{
	GENERATED_BODY()
	
public:

	virtual ETileType getTileType() override;
	virtual int32 getMovementCost() override;
};
