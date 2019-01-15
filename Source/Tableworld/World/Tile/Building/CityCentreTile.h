// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "World/Tile/Building/BuildableTile.h"
#include "CityCentreTile.generated.h"

class AHaulerCreature;
class AInventoryTile;

UCLASS()
class TABLEWORLD_API ACityCentreTile : public ABuildableTile
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CityCentre")
	int32 InfluenceRadius = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CityCentre")
	int32 HaulerAmount = 2;

	//The delay whenever the building checks for harvesters with inventory
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CityCentre")
	float RescourceCheckTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CityCentre")
	TSubclassOf<AHaulerCreature> HaulerClass = nullptr;

	virtual void StartWork() override;

	virtual int32 getBuildGridRadius() override;
	virtual bool InInfluenceRange(int32 X, int32 Y, FVector2D Size);

	void OnRescourceCheck();

	void OnHaulCompleted(AHaulerCreature* nHauler);

	AInventoryTile* getValidHaulGoal(FVector2D& InTile, FVector2D& OutTile);

protected:

	TArray<AHaulerCreature*> Workers;

	FTimerHandle RescourceCheckTimer;
};
