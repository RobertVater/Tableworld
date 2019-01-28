// Copyright by Robert Vater (Gunschlinger)

#include "HumanAnimBlueprint.h"
#include "Misc/TableHelper.h"

void UHumanAnimBlueprint::UpdateStatus(ECreatureStatus NewStatus)
{
	CurrentStatus = NewStatus;
}

ECreatureStatus UHumanAnimBlueprint::getCreatureStatus()
{
	return CurrentStatus;
}
