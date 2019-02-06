// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Misc/TableHelper.h"
#include "HumanAnimBlueprint.generated.h"

class AHumanCreature;

UCLASS(Blueprintable)
class TABLEWORLD_API UHumanAnimBlueprint : public UAnimInstance
{
	GENERATED_BODY()
	
public:

	virtual void NativeBeginPlay() override;

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe),Category = "Setter")
	void UpdateStatus(ECreatureStatus NewStatus);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = "Event")
	void PlayHarvestEffects();

	UFUNCTION(BlueprintCallable,BlueprintPure, meta = (BlueprintThreadSafe), Category = "Getter")
	ECreatureStatus getCreatureStatus();

protected:

	ECreatureStatus CurrentStatus = ECreatureStatus::Idle;
	AHumanCreature* HumanRef = nullptr;
};
