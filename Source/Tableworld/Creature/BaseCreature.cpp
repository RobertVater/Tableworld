// Copyright by Robert Vater (Gunschlinger)

#include "BaseCreature.h"
#include "Kismet/GameplayStatics.h"
#include "Core/TableGamemode.h"

ABaseCreature::ABaseCreature()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ABaseCreature::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseCreature::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(PathPoints.Num() > 0)
	{
		if(!PathPoints.IsValidIndex(CurrentPathIndex))
		{
			PathPoints.Empty();
			CurrentPathIndex = 0;
			MinDistance = 0.0f;

			OnMoveCompleted();
			return;
		}

		FVector CurrentMoveTarget = PathPoints[CurrentPathIndex];
		FVector Dir = -(GetActorLocation() - CurrentMoveTarget);
		Dir.Normalize();

		FVector CurrentLoc = GetActorLocation();
		CurrentLoc += Dir * (MovementSpeed * DeltaTime);

		SetActorLocation(CurrentLoc);

		if(GetActorLocation().Equals(CurrentMoveTarget, MinDistance))
		{
			CurrentPathIndex++;
			return;
		}
	}
}

void ABaseCreature::SimpleMoveTo(FVector TargetLocation, float nMinDistance /*= 50.0f*/)
{
	CurrentPathIndex = 0,
	
	PathPoints.Empty();
	PathPoints.Add(TargetLocation);

	MinDistance = nMinDistance;
}

void ABaseCreature::OnMoveCompleted()
{

}

ATableGamemode* ABaseCreature::getGamemode()
{
	if(!GM)
	{
		GM = Cast<ATableGamemode>(UGameplayStatics::GetGameMode(this));
	}

	return GM;
}

UTileData* ABaseCreature::getStandingTile()
{
	if(getGamemode())
	{
		int32 X = (int32)(GetActorLocation().X / 100);
		int32 Y = (int32)(GetActorLocation().Y / 100);

		return getGamemode()->getTile(X, Y);
	}

	return nullptr;
}

