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

void AHarvesterCreature::OnPlayHarvestEffect()
{
	PlayHarvestEffect();
}

void AHarvesterCreature::GiveHarvestJob(UTileData* nHarvestTile)
{
	HarvestTile = nHarvestTile;
	SetCreatureStatus(ECreatureStatus::GoingToWork);

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

		SetCreatureStatus(ECreatureStatus::ReturningGoods);
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
			SetCreatureStatus(ECreatureStatus::Harvesting);

			SetRotationGoal( (GetActorLocation() - getHarvestTile()->getWorldCenter()).Rotation().Yaw + 90.0f );
			
			float HarvestTime = FMath::RandRange(getHarvesterTile()->HarvestTime / 2.0f, getHarvesterTile()->HarvestTime);
			GetWorldTimerManager().SetTimer(HarvestTimer, this, &AHarvesterCreature::OnHarvest, HarvestTime, false);
		}
	}
}

void AHarvesterCreature::PlayHarvestEffect()
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

		SetCreatureStatus(ECreatureStatus::ReturningGoods);

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

void AHarvesterCreature::LoadData(FTableSaveHarvesterCreature Data)
{
	DebugWarning("Load Worker Data!");

	//Load the saved path of the creature
	HomeTile = Data.HomeTile;
	PathPoints = Data.Path;
	LastCalculatedPath = UTableHelper::Convert2DVectorArrayToTileArray(Data.LastCalcPath);
	CurrentPathIndex = Data.PathIndex;
	MinDistance = Data.MinDinstance;

	//Load the exact location and the status of the creature
	SetActorLocation(Data.Location);
	DrawDebugPoint(GetWorld(), Data.Location, 10, FColor::Orange, false, 10, 0);
	DrawDebugPoint(GetWorld(), GetActorLocation(), 10, FColor::Green, false, 10, 0);
	SetCreatureStatus(Data.Status);

	bHasHarvested = Data.bHasCollected;

	//Get the HarvestTile
	if(Data.bHasHarvestTile)
	{
		HarvestTile = getGamemode()->getTile(Data.HarvestTileX, Data.HarvestTileY);
	}

	//Set the HarvestTimer
	if(Data.HarvestTimer > 0)
	{
		GetWorldTimerManager().SetTimer(HarvestTimer, this, &AHarvesterCreature::OnHarvest, Data.HarvestTimer, false);
	}
}

FTableSaveHarvesterCreature AHarvesterCreature::getSaveData()
{
	FTableSaveHarvesterCreature Data;

	Data.HomeTile = HomeTile;
	Data.Path = PathPoints;
	Data.LastCalcPath = UTableHelper::ConvertTileArrayTo2DVectorArray(LastCalculatedPath);
	Data.MinDinstance = MinDistance;
	Data.PathIndex = CurrentPathIndex;

	Data.Location = GetActorLocation();
	Data.Status = getStatus();

	Data.HarvestTimer = GetWorldTimerManager().GetTimerRemaining(HarvestTimer);
	Data.bHasCollected = bHasHarvested;

	if(HarvestTile)
	{
		Data.HarvestTileX = HarvestTile->getX();
		Data.HarvestTileY = HarvestTile->getY();
		Data.bHasHarvestTile = true;
	}

	return Data;
}
