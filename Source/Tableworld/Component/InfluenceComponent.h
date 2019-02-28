// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InfluenceComponent.generated.h"

class ABuildableTile;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TABLEWORLD_API UInfluenceComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInfluenceComponent();	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Influence")
	int32 InfluenceRange = 25;

	void ShowInfluenceRadius();
	void HideInfluenceRadius();

	ABuildableTile* getParentBuilding();
	int32 getX();
	int32 getY();

protected:

	ABuildableTile* ParentBuilding = nullptr;
};
