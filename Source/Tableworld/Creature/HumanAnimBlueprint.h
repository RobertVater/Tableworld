// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Misc/TableHelper.h"
#include "HumanAnimBlueprint.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class TABLEWORLD_API UHumanAnimBlueprint : public UAnimInstance
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe),Category = "Setter")
	void UpdateStatus(ECreatureStatus NewStatus);

	UFUNCTION(BlueprintCallable,BlueprintPure, meta = (BlueprintThreadSafe), Category = "Getter")
	ECreatureStatus getCreatureStatus();

protected:

	ECreatureStatus CurrentStatus = ECreatureStatus::Idle;
};
