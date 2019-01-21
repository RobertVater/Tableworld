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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	UAnimationAsset* Work = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	UAnimationAsset* IdleWood = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	UAnimationAsset* WalkWood = nullptr;

	//The time when we should play special effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	float WorkTime = 0.6f;

	//The max time it takes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	float MaxWorkTime = 1.2f;

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

	virtual UAnimationAsset* getIdleAnimation() override;
	virtual UAnimationAsset* getWalkAnimation() override;

protected:

	FTimerHandle HarvestTimer;
	FTimerHandle HarvestEffectsTimer;

	bool bHasHarvested = false;
	AHarvesterTile* HarvesterBuilding = nullptr;

	UTileData* HarvestTile = nullptr;
};
