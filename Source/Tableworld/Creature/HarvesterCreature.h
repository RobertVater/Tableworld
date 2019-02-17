// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "WorkerCreature.h"
#include "HarvesterCreature.generated.h"

class AHarvesterTile;
class UTileData;

UCLASS()
class TABLEWORLD_API AHarvesterCreature : public AWorkerCreature
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

	virtual void OnPlayHarvestEffect() override;

	void GiveHarvestJob(UTileData* nHarvestTile, EItem HarvestItem);
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
	EItem getHarvestItem();
	bool HasTileStillRescources();
	bool hasHarvested();

	virtual UAnimationAsset* getIdleAnimation() override;
	virtual UAnimationAsset* getWalkAnimation() override;

protected:

	FTimerHandle HarvestTimer;

	EItem HarvestedItem = EItem::None;
	bool bHasHarvested = false;

	AHarvesterTile* HarvesterBuilding = nullptr;

	UTileData* HarvestTile = nullptr;
};
