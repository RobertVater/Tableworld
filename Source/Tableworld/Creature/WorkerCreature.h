// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "Creature/HumanCreature.h"
#include "WorkerCreature.generated.h"


UCLASS()
class TABLEWORLD_API AWorkerCreature : public AHumanCreature
{
	GENERATED_BODY()

public:

	void Init(ABuildableTile* nParentBuilding, FVector2D nHomeTile);

protected:

	ABuildableTile* ParentBuilding = nullptr;
};
