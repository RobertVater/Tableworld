// Copyright by Robert Vater (Gunschlinger)

#include "BaseCreature.h"
#include "Kismet/GameplayStatics.h"
#include "Core/TableGamemode.h"
#include "World/TableWorldTable.h"
#include "World/Tile/TileData.h"
#include "DrawDebugHelpers.h"
#include "HeapSort.h"

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
		for(int32 i = 0; i < PathPoints.Num(); i++)
		{
			DrawDebugString(GetWorld(), PathPoints[i], FString::FromInt(i), NULL, FColor::Cyan, 0, true);
		}
		
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

		DrawDebugLine(GetWorld(), GetActorLocation(), CurrentMoveTarget, FColor::Green, false, 0, 0, 2.0f);
		DrawDebugPoint(GetWorld(), CurrentMoveTarget, 10.0f, FColor::Red, false, 0, 0);
		DrawDebugString(GetWorld(), CurrentMoveTarget, FString::FromInt(CurrentPathIndex), NULL, FColor::White, 0, true);

		FVector CurrentLoc = GetActorLocation();
		CurrentLoc += Dir * (MovementSpeed * DeltaTime);

		SetActorLocation(CurrentLoc);
		float Dist = (GetActorLocation() - CurrentMoveTarget).Size2D();
		DrawDebugString(GetWorld(), GetActorLocation(), FString::SanitizeFloat(Dist), NULL, FColor::White, 0, true);

		if(Dist <= MinDistance)
		{
			CurrentPathIndex++;
			return;
		}
	}
}

void ABaseCreature::SimpleMoveTo(FVector TargetLocation, float nMinDistance /*= 50.0f*/)
{
	CurrentPathIndex = 0;
	
	PathPoints.Empty();
	PathPoints.Add(TargetLocation);

	MinDistance = nMinDistance;
}

void ABaseCreature::PathMoveTo(UTileData* TargetTile, float nMinDistance /*= 50.0f*/)
{
	if (TargetTile) 
	{
		if (getGamemode()) 
		{
			if (getGamemode()->getTable()) 
			{
				CurrentPathIndex = 0;
				MinDistance = nMinDistance;
				PathPoints.Empty();

				LastCalculatedPath = getGamemode()->getTable()->FindPath(FVector2D(getStandingTile()->getX(), getStandingTile()->getY()), FVector2D(TargetTile->getX(), TargetTile->getY()), TArray<ETileType>());
				for(int32 i = LastCalculatedPath.Num()-1; i > 0; i--)
				{
					UTileData* Tile = LastCalculatedPath[i];
					if(Tile)
					{
						PathPoints.Add(Tile->getWorldCenter());
					}
				}
			}
		}
	}
}

void ABaseCreature::RoadMoveTo(UTileData* TargetTile, float nMinDistance /*= 50.0f*/)
{
	if (TargetTile)
	{
		if (getGamemode())
		{
			if (getGamemode()->getTable())
			{
				CurrentPathIndex = 0;
				MinDistance = nMinDistance;
				PathPoints.Empty();

				LastCalculatedPath = getGamemode()->getTable()->FindPathRoad(getStandingTile(), TargetTile, false);
				for (int32 i = 0; i < LastCalculatedPath.Num(); i++)
				{
					UTileData* Tile = LastCalculatedPath[i];
					if (Tile)
					{
						PathPoints.Add(Tile->getWorldCenter());
					}
				}
			}
		}
	}
}

void ABaseCreature::RetracePath()
{
	CurrentPathIndex = 0;
	MinDistance = 50.0f;
	PathPoints.Empty();

	TArray<UTileData*> ReversePath = LastCalculatedPath;
	Algo::Reverse(ReversePath);

	for (int32 i = 0; i < ReversePath.Num(); i++)
	{
		UTileData* Tile = ReversePath[i];
		if (Tile)
		{
			PathPoints.Add(Tile->getWorldCenter());
		}
	}
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

