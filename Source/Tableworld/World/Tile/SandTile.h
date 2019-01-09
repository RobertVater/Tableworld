// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "World/Tile/TileData.h"
#include "SandTile.generated.h"

/**
 * 
 */
UCLASS()
class TABLEWORLD_API USandTile : public UTileData
{
	GENERATED_BODY()
	
public:

	virtual ETileType getTileType() override;
};
