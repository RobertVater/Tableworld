// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Misc/TableHelper.h"
#include "BaseCreature.generated.h"

class ATableGamemode;
class UTileData;
class UAnimationAsset;

UCLASS()
class TABLEWORLD_API ABaseCreature : public AActor
{
	GENERATED_BODY()
	
public:	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	float MovementSpeed = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	float RotationSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	UAnimationAsset* Idle = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	UAnimationAsset* Walk = nullptr;

	ABaseCreature();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void DeactivateCreature();
	virtual void ActivateCreature();

	virtual void SetRotationGoal(float NewGoal);


	void SetAnimation(UAnimationAsset* Anim);

	virtual void SimpleMoveTo(FVector TargetLocation, float nMinDistance = 50.0f);

	virtual void PathMoveTo(UTileData* TargetTile, float nMinDistance = 25.0f);
	virtual void RoadMoveTo(UTileData* TargetTile, float nMinDistance = 25.0f);
	virtual void RetracePath();


	virtual void OnMoveCompleted();

	ATableGamemode* getGamemode();
	UTileData* getStandingTile();

	virtual UAnimationAsset* getIdleAnimation();
	virtual UAnimationAsset* getWalkAnimation();
	ECreatureStatus getStatus();

protected:

	ECreatureStatus CreatureStatus = ECreatureStatus::Idle;

	ATableGamemode* GM = nullptr;

	float RotationGoal = 0.0f;

	float MinDistance = 0.0f;
	int32 CurrentPathIndex = 0;
	TArray<FVector> PathPoints;

	//The last path we calculated
	TArray<UTileData*> LastCalculatedPath;

	FVector2D HomeTile;
};
