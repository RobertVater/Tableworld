// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "World/Tile/Building/BuildableTile.h"
#include "CityCentreTile.generated.h"

/**
 * 
 */
UCLASS()
class TABLEWORLD_API ACityCentreTile : public ABuildableTile
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CityCentre")
	int32 InfluenceRadius = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CityCentre")
	int32 BuildingLimit = 10;

	virtual int32 getBuildGridRadius() override;
};
