// Copyright by Robert Vater (Gunschlinger)

#include "HumanAnimBlueprint.h"
#include "Misc/TableHelper.h"

void UHumanAnimBlueprint::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	HumanRef = Cast<AHumanCreature>(GetOwningActor());
}

void UHumanAnimBlueprint::UpdateStatus(ECreatureStatus NewStatus)
{
	CurrentStatus = NewStatus;
}

void UHumanAnimBlueprint::PlayHarvestEffects()
{
	if(HumanRef)
	{
		HumanRef->OnPlayHarvestEffect();
	}
}

ECreatureStatus UHumanAnimBlueprint::getCreatureStatus()
{
	return CurrentStatus;
}
