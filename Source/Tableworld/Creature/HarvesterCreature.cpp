// Copyright by Robert Vater (Gunschlinger)

#include "HarvesterCreature.h"
#include "World/Tile/TileData.h"
#include "World/Tile/Building/HarvesterBuilding.h"
#include "Core/TableGamemode.h"
#include "World/TableWorldTable.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Misc/TableHelper.h"


void AHarvesterCreature::OnPlayHarvestEffect()
{
	PlayHarvestEffect();
}

void AHarvesterCreature::GiveHarvestJob(UTileData* nHarvestTile, EItem HarvestItem)
{
	HarvestedItem = HarvestItem;
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

	//Play the wobble animation
	if (getGamemode())
	{
		if (getGamemode()->getTable())
		{
			getGamemode()->getTable()->AddRescourceWobble(HarvestTile->getTileRescources(), HarvestTile->getTileRescourceIndex(), 1.0f, HarvestTile->getParentChunk());

			if (HarvestParticles)
			{
				if (HarvestTile)
				{
					FTransform ResTransform = HarvestTile->getRescourceTransform();

					FVector Location = ResTransform.GetLocation();

					if(HarvestTile->getTileRescources() == ETileRescources::None)
					{
						Location = Mesh->GetSocketLocation("Pickaxe_End");
					}

					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HarvestParticles, Location, FRotator(0.0f, FMath::RandRange(-90.0f, 90.0f), 0.0f), true);
				}
			}
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
	if(!HarvesterBuilding)
	{
		HarvesterBuilding = Cast<AHarvesterTile>(ParentBuilding);
	}
	
	return HarvesterBuilding;
}

UTileData* AHarvesterCreature::getHarvestTile()
{
	return HarvestTile;
}

EItem AHarvesterCreature::getHarvestItem()
{
	return HarvestedItem;
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
