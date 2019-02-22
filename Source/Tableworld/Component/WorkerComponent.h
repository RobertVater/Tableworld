// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WorkerComponent.generated.h"

class AWorkerCreature;
class ABuildableTile;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TABLEWORLD_API UWorkerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Worker")
	uint8 MaxWorkerCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Worker")
	TSubclassOf<AWorkerCreature> WorkerClass = nullptr;

	UWorkerComponent();	

	virtual void Init(ABuildableTile* nParentBuilding);
	virtual AWorkerCreature* SpawnWorker();
	virtual void SpawnAllWorkers();

	uint8 getMaxWorkerCount();
	AWorkerCreature* getDeactivatedWorker();
	TArray<AWorkerCreature*> getWorkers();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	TArray<FName> getWorkerNames();

protected:
	
	ABuildableTile* ParentBuilding = nullptr;
	FVector WorkerSpawnLocation = FVector::ZeroVector;

	TArray<AWorkerCreature*> Workers;
};
