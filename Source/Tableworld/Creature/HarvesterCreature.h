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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	USoundBase* HarvestSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	UParticleSystem* HarvestParticles = nullptr;

	virtual void Create(FVector2D nCreationTileLocation, AHarvesterTile* nHarvesterBuilding);

	void GiveHarvestJob(UTileData* nHarvestTile);
	void GiveReturnJob();

	virtual void OnMoveCompleted() override;
	void ResetHasHarvested();

	//Harvesting
	void StartHarvesting();
	void OnHarvestEffect();

	UFUNCTION()
	void OnHarvest();

	AHarvesterTile* getHarvesterTile();
	UTileData* getHarvestTile();
	bool HasTileStillRescources();
	bool hasHarvested();

protected:

	FTimerHandle HarvestTimer;
	FTimerHandle HarvestEffectsTimer;

	bool bHasHarvested = false;
	AHarvesterTile* HarvesterBuilding = nullptr;

	UTileData* HarvestTile = nullptr;
};
