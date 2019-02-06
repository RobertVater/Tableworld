// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "Creature/BaseCreature.h"
#include "HumanCreature.generated.h"

/**
 * 
 */
UCLASS()
class TABLEWORLD_API AHumanCreature : public ABaseCreature
{
	GENERATED_BODY()
	
public:

	//Hooks for Animation Events from the HumanAnimationBlueprint
	UFUNCTION(BlueprintCallable,Category = "Animation Events")
	virtual void OnPlayHarvestEffect();
};
