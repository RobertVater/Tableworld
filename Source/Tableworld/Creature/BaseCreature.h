// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Misc/TableHelper.h"
#include "Interface/InfoPanelInterface.h"
#include "BaseCreature.generated.h"

class ATableGamemode;
class UTileData;
class UAnimationAsset;
class UHumanAnimBlueprint;

UCLASS()
class TABLEWORLD_API ABaseCreature : public AActor, public IInfoPanelInterface
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

	virtual void Create();

	virtual void DeactivateCreature();
	virtual void ActivateCreature();

	virtual void OnActivate();
	virtual void OnDeactivate();

	virtual void SetRotationGoal(float NewGoal);

	void SetCreatureStatus(ECreatureStatus NewStatus);
	void UpdateCreatureStatus();

	virtual void StopMovement();
	virtual void SimpleMoveTo(FVector TargetLocation, float nMinDistance = 50.0f);

	virtual void PathMoveTo(UTileData* TargetTile, float nMinDistance = 25.0f);
	virtual void RoadMoveTo(UTileData* TargetTile, float nMinDistance = 25.0f);
	virtual void RetracePath();

	virtual void OnMoveCompleted();

	ATableGamemode* getGamemode();
	UHumanAnimBlueprint* getAnimationBlueprint();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	UTileData* getStandingTile();

	virtual UAnimationAsset* getIdleAnimation();
	virtual UAnimationAsset* getWalkAnimation();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	ECreatureStatus getStatus();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	virtual FName getReadableStatus();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	virtual FName getCreatureName();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	virtual bool isMale();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	virtual uint8 getSkinIndex();

	//Interface
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interface")
	FTableInfoPanel getInfoPanelData();
	virtual FTableInfoPanel getInfoPanelData_Implementation();



protected:

	int32 NameIndex = 0;
	uint8 SkinIndex = 0;
	bool bIsMale = true;

	ECreatureStatus CreatureStatus = ECreatureStatus::Idle;

	ATableGamemode* GM = nullptr;
	UHumanAnimBlueprint* AnimationBlueprint = nullptr;

	float RotationGoal = 0.0f;

	float MinDistance = 0.0f;
	int32 CurrentPathIndex = 0;
	TArray<FVector> PathPoints;

	//The last path we calculated
	TArray<UTileData*> LastCalculatedPath;

	FVector2D HomeTile;
};
