// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "Creature/BaseCreature.h"
#include "HarvesterCreature.generated.h"

class AHarvesterTile;
class UTileData;

UCLASS()
class TABLEWORLD_API AHarvesterCreature : public ABaseCreature
{
	GENERATED_BODY()
	
public:

	virtual void Create(FVector2D nCreationTileLocation, AHarvesterTile* nHarvesterBuilding);

	void GiveHarvestJob(UTileData* nHarvestTile);
	void GiveReturnJob();

	virtual void OnMoveCompleted() override;

	//Harvesting
	void StartHarvesting();
	void OnHarvest();

	AHarvesterTile* getHarvesterTile();
	UTileData* getHarvestTile();
	bool HasTileStillRescources();

protected:

	FTimerHandle HarvestTimer;

	AHarvesterTile* HarvesterBuilding = nullptr;
	UTileData* HarvestTile = nullptr;
};
