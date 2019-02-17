// Copyright by Robert Vater (Gunschlinger)

#include "WorkerCreature.h"

void AWorkerCreature::Init(ABuildableTile* nParentBuilding, FVector2D nHomeTile)
{
	ParentBuilding = nParentBuilding;
	HomeTile = nHomeTile;
}
