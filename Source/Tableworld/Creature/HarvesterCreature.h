// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "Creature/BaseCreature.h"
#include "HumanCreature.h"
#include "HarvesterCreature.generated.h"

class AHarvesterTile;
class UTileData;

UCLASS()
class TABLEWORLD_API AHarvesterCreature : public AHumanCreature
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
	UAnimationAsset* WalkWood = nullptr;

	virtual void Create(FVector2D nCreationTileLocation, AHarvesterTile* nHarvesterBuilding);
	virtual void OnPlayHarvestEffect() override;

	void GiveHarvestJob(UTileData* nHarvestTile);
	void GiveReturnJob();

	virtual void OnMoveCompleted() override;
	void ResetHasHarvested();

	//Harvesting
	void StartHarvesting();

	UFUNCTION(BlueprintCallable, Category = "Effects")
	void PlayHarvestEffect();

	UFUNCTION()
	void OnHarvest();

	AHarvesterTile* getHarvesterTile();
	UTileData* getHarvestTile();
	bool HasTileStillRescources();
	bool hasHarvested();

	virtual UAnimationAsset* getIdleAnimation() override;
	virtual UAnimationAsset* getWalkAnimation() override;

	void LoadData(FTableSaveHarvesterCreature Data);
	FTableSaveHarvesterCreature getSaveData();

protected:

	FTimerHandle HarvestTimer;

	bool bHasHarvested = false;
	AHarvesterTile* HarvesterBuilding = nullptr;

	UTileData* HarvestTile = nullptr;
};
