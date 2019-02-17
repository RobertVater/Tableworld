// Copyright by Robert Vater (Gunschlinger)

#include "WorkerComponent.h"
#include "Creature/WorkerCreature.h"
#include "World/Tile/Building/BuildableTile.h"

UWorkerComponent::UWorkerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWorkerComponent::Init(ABuildableTile* nParentBuilding)
{
	ParentBuilding = nParentBuilding;

	if(ParentBuilding)
	{
		WorkerSpawnLocation = ParentBuilding->getWorldCenter();
	}
}

AWorkerCreature* UWorkerComponent::SpawnWorker()
{
	if (WorkerClass)
	{
		AWorkerCreature* Worker = GetWorld()->SpawnActor<AWorkerCreature>(WorkerClass, WorkerSpawnLocation, FRotator::ZeroRotator);
		if(Worker)
		{
			Worker->Create();
			Worker->Init(ParentBuilding, FVector2D(ParentBuilding->getTileX(), ParentBuilding->getTileY()));
			Worker->DeactivateCreature();

			Workers.Add(Worker);
		}
	}

	return nullptr;
}

void UWorkerComponent::SpawnAllWorkers()
{
	for(int32 i = 0; i < MaxWorkerCount; i++)
	{
		SpawnWorker();
	}
}

uint8 UWorkerComponent::getMaxWorkerCount()
{
	return MaxWorkerCount;
}

AWorkerCreature* UWorkerComponent::getDeactivatedWorker()
{
	for(int32 i = 0; i < Workers.Num(); i++)
	{
		AWorkerCreature* Worker = Workers[i];
		if(Worker)
		{
			if(Worker->getStatus() == ECreatureStatus::Deactivated)
			{
				return Worker;
			}
		}
	}

	return nullptr;
}

TArray<AWorkerCreature*> UWorkerComponent::getWorkers()
{
	return Workers;
}
