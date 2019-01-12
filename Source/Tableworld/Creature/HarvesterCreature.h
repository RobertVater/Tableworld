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
	void ResetHasHarvested();

	//Harvesting
	void StartHarvesting();
	void OnHarvest();

	AHarvesterTile* getHarvesterTile();
	UTileData* getHarvestTile();
	bool HasTileStillRescources();
	bool hasHarvested();

protected:

	FTimerHandle HarvestTimer;

	bool bHasHarvested = false;
	AHarvesterTile* HarvesterBuilding = nullptr;

	UTileData* HarvestTile = nullptr;
	UTileData* ReturnTile = nullptr;
};
