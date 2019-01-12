// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseCreature.generated.h"

class ATableGamemode;
class UTileData;

UCLASS()
class TABLEWORLD_API ABaseCreature : public AActor
{
	GENERATED_BODY()
	
public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	float MovementSpeed = 100;

	ABaseCreature();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void SimpleMoveTo(FVector TargetLocation, float nMinDistance = 50.0f);

	virtual void OnMoveCompleted();

	ATableGamemode* getGamemode();
	UTileData* getStandingTile();

protected:

	ATableGamemode* GM = nullptr;

	float MinDistance = 0.0f;
	int32 CurrentPathIndex = 0;
	TArray<FVector> PathPoints;

	FVector2D HomeTile;
};
