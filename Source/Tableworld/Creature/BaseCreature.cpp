// Copyright by Robert Vater (Gunschlinger)

#include "BaseCreature.h"
#include "Kismet/GameplayStatics.h"
#include "Core/TableGamemode.h"
#include "World/TableWorldTable.h"
#include "World/Tile/TileData.h"
#include "DrawDebugHelpers.h"
#include "HeapSort.h"
#include "Kismet/KismetMathLibrary.h"
#include "HumanAnimBlueprint.h"

ABaseCreature::ABaseCreature()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
}

void ABaseCreature::BeginPlay()
{
	Super::BeginPlay();

	RotationGoal = GetActorRotation().Yaw;
	SetCreatureStatus(ECreatureStatus::Idle);
}

void ABaseCreature::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (UTableHelper::isDebug()) 
	{
		if (getStatus() == ECreatureStatus::Deactivated)
		{
			DrawDebugString(GetWorld(), GetActorLocation() + FVector(0, 0, 150), "Deactivated", NULL, FColor::Cyan, 0, true);
		}
	}

	if(PathPoints.Num() > 0)
	{
		if (UTableHelper::isDebug()) 
		{
			for (int32 i = 0; i < PathPoints.Num(); i++)
			{
				DrawDebugString(GetWorld(), PathPoints[i], FString::FromInt(i), NULL, FColor::Cyan, 0, true);
			}
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

		if (UTableHelper::isDebug()) 
		{
			DrawDebugLine(GetWorld(), GetActorLocation(), CurrentMoveTarget, FColor::Green, false, 0, 0, 2.0f);
			DrawDebugPoint(GetWorld(), CurrentMoveTarget, 10.0f, FColor::Red, false, 0, 0);
			DrawDebugString(GetWorld(), CurrentMoveTarget, FString::FromInt(CurrentPathIndex), NULL, FColor::White, 0, true);
		}

		//Rotate to pathpoint
		SetRotationGoal(Dir.Rotation().Yaw - 90.0f);

		FVector CurrentLoc = GetActorLocation();
		CurrentLoc += Dir * (MovementSpeed * DeltaTime);

		SetActorLocation(CurrentLoc);
		float Dist = (GetActorLocation() - CurrentMoveTarget).Size2D();
		
		if (UTableHelper::isDebug()) 
		{
			DrawDebugString(GetWorld(), GetActorLocation(), FString::SanitizeFloat(Dist), NULL, FColor::White, 0, true);
		}

		if(Dist <= MinDistance)
		{
			CurrentPathIndex++;
			return;
		}
	}

	float YawLerp = UKismetMathLibrary::RLerp(GetActorRotation(), FRotator(0.0f, RotationGoal, 0.0f), RotationSpeed * DeltaTime, true).Yaw;
	SetActorRotation(FRotator(0.0f, YawLerp, 0.0f));
}

void ABaseCreature::Create()
{
	bIsMale = FMath::RandBool();
}

void ABaseCreature::DeactivateCreature()
{
	SetCreatureStatus(ECreatureStatus::Deactivated);

	Mesh->SetVisibility(false);
	PathPoints.Empty();
	MinDistance = 0.0f;
	CurrentPathIndex = 0;

	OnDeactivate();
}

void ABaseCreature::ActivateCreature()
{
	Mesh->SetVisibility(true);
	SetCreatureStatus(ECreatureStatus::Idle);

	OnActivate();
}

void ABaseCreature::OnActivate()
{
	
}

void ABaseCreature::OnDeactivate()
{

}

void ABaseCreature::SetRotationGoal(float NewGoal)
{
	RotationGoal = NewGoal;
}

void ABaseCreature::SetCreatureStatus(ECreatureStatus NewStatus)
{
	CreatureStatus = NewStatus;

	if(getAnimationBlueprint())
	{
		getAnimationBlueprint()->UpdateStatus(CreatureStatus);
	}

}

void ABaseCreature::UpdateCreatureStatus()
{
	if(getStatus() == ECreatureStatus::Deactivated)
	{
		DeactivateCreature();
		return;
	}

	Mesh->SetVisibility(true);
}

void ABaseCreature::StopMovement()
{
	CurrentPathIndex = 0;

	PathPoints.Empty();
	MinDistance = 0.0f;
}

void ABaseCreature::SimpleMoveTo(FVector TargetLocation, float nMinDistance /*= 50.0f*/)
{
	StopMovement();

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
				StopMovement();
				MinDistance = nMinDistance;

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
				StopMovement();
				MinDistance = nMinDistance;

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
	StopMovement();
	MinDistance = 25.0f;

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

UHumanAnimBlueprint* ABaseCreature::getAnimationBlueprint()
{
	if(!AnimationBlueprint)
	{
		if(Mesh)
		{
			AnimationBlueprint = Cast<UHumanAnimBlueprint>(Mesh->GetAnimInstance());
		}
	}

	return AnimationBlueprint;
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

UAnimationAsset* ABaseCreature::getIdleAnimation()
{
	return Idle;
}

UAnimationAsset* ABaseCreature::getWalkAnimation()
{
	return Walk;
}

ECreatureStatus ABaseCreature::getStatus()
{
	return CreatureStatus;
}

FName ABaseCreature::getReadableStatus()
{
	switch(getStatus())
	{
	case ECreatureStatus::Deactivated: return "";
	case ECreatureStatus::GoingToWork: return "Going to work";
	case ECreatureStatus::Harvesting: return "Gathering";
	case ECreatureStatus::Idle: return "Idle";
	case ECreatureStatus::ReturningGoods: return "Returning Items";
	case ECreatureStatus::Wandering: return "Wandering around";
	}

	return FName();
}

FName ABaseCreature::getCreatureName()
{
	return FName();
}

bool ABaseCreature::isMale()
{
	return bIsMale;
}

uint8 ABaseCreature::getSkinIndex()
{
	return SkinIndex;
}

FTableInfoPanel ABaseCreature::getInfoPanelData_Implementation()
{
	FTableInfoPanel Panel;

	FTableInfo_Text NameText;
	NameText.RawText = FText::FromName(getCreatureName());
	NameText.Size = 25;
	Panel.DetailText.Add(NameText);

	FTableInfo_Text SatusText;
	SatusText.RawText = FText::FromName(getReadableStatus());
	SatusText.Size = 15;
	Panel.DetailText.Add(SatusText);

	FTableInfo_Text GenderText;
	GenderText.Size = 20;
	GenderText.RawText = FText::FromString(bIsMale ? "Male" : "Female");
	Panel.InfoText.Add(GenderText);

	Panel.WorldContext = this;
	return Panel;
}

