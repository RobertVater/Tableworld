// Copyright by Robert Vater (Gunschlinger)

#include "HarvesterCreature.h"
#include "World/Tile/TileData.h"
#include "World/Tile/Building/HarvesterTile.h"
#include "Core/TableGamemode.h"
#include "World/TableWorldTable.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Misc/TableHelper.h"

void AHarvesterCreature::Create(FVector2D nCreationTileLocation, AHarvesterTile* nHarvesterBuilding)
{
	HomeTile = nCreationTileLocation;
	HarvesterBuilding = nHarvesterBuilding;

	DebugWarning("Created! " + HomeTile.ToString());
}

void AHarvesterCreature::GiveHarvestJob(UTileData* nHarvestTile)
{
	HarvestTile = nHarvestTile;

	if(HarvestTile)
	{
		//Move to the tile
		SimpleMoveTo(HarvestTile->getWorldCenter());
	}
}

void AHarvesterCreature::GiveReturnJob()
{
	//Clear all times we might have
	GetWorldTimerManager().ClearAllTimersForObject(this);
	
	if(getHarvesterTile())
	{
		FVector Loc;
		Loc.X = getHarvesterTile()->getTileX() * 100 + 50;
		Loc.Y = getHarvesterTile()->getTileY() * 100 + 50;
		Loc.Z = 0.0f;

		HomeTile.X = getHarvesterTile()->getTileX();
		HomeTile.Y = getHarvesterTile()->getTileY();

		DrawDebugPoint(GetWorld(), Loc, 10, FColor::Orange, false, 999, 0);

		SimpleMoveTo(Loc);
	}
	else
	{
		//Our Owner is gone. Suicide!
		Destroy();
	}
}

void AHarvesterCreature::OnMoveCompleted()
{
	Super::OnMoveCompleted();

	//Check if we reached the rescource of the home tile
	UTileData* OurTile = getStandingTile();
	if (OurTile) 
	{
		//Rescourse
		if (getHarvestTile()) 
		{
			if (OurTile->getX() == getHarvestTile()->getX())
			{
				if (OurTile->getY() == getHarvestTile()->getY())
				{
					//Try to harvest the tile
					StartHarvesting();
					return;
				}
			}
		}

		if(getHarvesterTile())
		{
			if (OurTile->getX() == HomeTile.X)
			{
				if (OurTile->getY() == HomeTile.Y)
				{
					getHarvesterTile()->OnWorkerReturn(this);
					return;
				}
			}
		}
	}
}

void AHarvesterCreature::ResetHasHarvested()
{
	bHasHarvested = false;
}

void AHarvesterCreature::StartHarvesting()
{	
	if (!GetWorldTimerManager().IsTimerActive(HarvestTimer))
	{
		if (getHarvesterTile())
		{
			CreatureStatus = ECreatureStatus::Harvesting;
			SetAnimation(Work);

			SetRotationGoal( (GetActorLocation() - getHarvestTile()->getWorldCenter()).Rotation().Yaw + 90.0f );
			
			float HarvestTime = FMath::RandRange(getHarvesterTile()->HarvestTime / 2.0f, getHarvesterTile()->HarvestTime);
			GetWorldTimerManager().SetTimer(HarvestTimer, this, &AHarvesterCreature::OnHarvest, HarvestTime, false);

			GetWorldTimerManager().SetTimer(HarvestEffectsTimer, this, &AHarvesterCreature::OnHarvestEffect, MaxWorkTime, true, WorkTime);
		}
	}
}

void AHarvesterCreature::OnHarvestEffect()
{
	if (HarvestSound) 
	{
		UGameplayStatics::PlaySoundAtLocation(this, HarvestSound, GetActorLocation());
	}

	if(HarvestParticles)
	{
		FVector Start = GetActorLocation();
		FVector Dir = GetActorForwardVector() * 50.0f;
		FVector End = Dir + Start;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HarvestParticles, End, FRotator::ZeroRotator, true);
	}

	//Play the wobble animation
	if (getGamemode())
	{
		if (getGamemode()->getTable())
		{
			getGamemode()->getTable()->AddRescourceWobble(HarvestTile->getTileRescources(), HarvestTile->getTileRescourceIndex(), 1.0f);
		}
	}
}

void AHarvesterCreature::OnHarvest()
{
	GetWorldTimerManager().ClearTimer(HarvestEffectsTimer);
	
	if(getHarvestTile())
	{
		if(getGamemode())
		{
			if(getGamemode()->getTable())
			{
				//Harvest the tile
				bool bDestroyed = getGamemode()->getTable()->HarvestRescource(getHarvestTile(), 1);
				bHasHarvested = true;
			}
		}

		CreatureStatus = ECreatureStatus::ReturningGoods;

		//Return home
		GiveReturnJob();
	}
}

AHarvesterTile* AHarvesterCreature::getHarvesterTile()
{
	return HarvesterBuilding;
}

UTileData* AHarvesterCreature::getHarvestTile()
{
	return HarvestTile;
}

bool AHarvesterCreature::HasTileStillRescources()
{
	if(getHarvestTile())
	{
		if(getHarvestTile()->getTileRescources() != ETileRescources::None)
		{
			return true;
		}
	}

	return false;
}

bool AHarvesterCreature::hasHarvested()
{
	return bHasHarvested;
}

UAnimationAsset* AHarvesterCreature::getIdleAnimation()
{
	if(hasHarvested())
	{
		return IdleWood;
	}

	return Idle;
}

UAnimationAsset* AHarvesterCreature::getWalkAnimation()
{
	if(hasHarvested())
	{
		return WalkWood;
	}

	return Walk;
}
