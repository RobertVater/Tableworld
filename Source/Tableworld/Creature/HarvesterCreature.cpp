// Copyright by Robert Vater (Gunschlinger)

#include "HarvesterCreature.h"
#include "World/Tile/TileData.h"
#include "World/Tile/Building/HarvesterTile.h"
#include "Core/TableGamemode.h"
#include "World/TableWorldTable.h"

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
		HarvestTile->DebugHighlightTile(1.0f);

		//Move to the tile
		SimpleMoveTo(HarvestTile->getWorldCenter());
	}
}

void AHarvesterCreature::GiveReturnJob()
{
	if(getHarvesterTile())
	{
		if (getHarvesterTile()->getTable()) 
		{
			UTileData* BestTile = nullptr;
			TArray<UTileData*> TilesAroundBuilding = getHarvesterTile()->getTilesAroundUs();
			for (int32 i = 0; i < TilesAroundBuilding.Num(); i++)
			{
				UTileData* Tile = TilesAroundBuilding[i];
				if (Tile != nullptr)
				{
					if (!Tile->IsBlocked())
					{
						if(!BestTile)
						{
							BestTile = Tile;
							continue;
						}
						
						int32 Distance = getHarvesterTile()->getTable()->getDistance(getStandingTile(), BestTile);
						int32 NewDistance = getHarvesterTile()->getTable()->getDistance(getStandingTile(), Tile);
						
						if(Distance > NewDistance)
						{
							BestTile = Tile;
						}
					}
				}
			}

			if(!BestTile)
			{
				HomeTile.X = getHarvesterTile()->getTileX();
				HomeTile.Y = getHarvesterTile()->getTileY();

				//We cannot find a way home. Just return to the home tile
				SimpleMoveTo(FVector(getHarvesterTile()->getTileX() * 100 + 50, getHarvesterTile()->getTileY() * 100 + 50, 0));
			}else
			{
				//Update our home tile
				HomeTile.X = BestTile->getX();
				HomeTile.Y = BestTile->getY();

				BestTile->DebugHighlightTile(1.0f);
				SimpleMoveTo(BestTile->getWorldCenter());
			}
		}
	}
	else
	{
		//Our Owner is gone. Suicide!
		Destroy();
	}
}

void AHarvesterCreature::OnMoveCompleted()
{
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
	if(!HasTileStillRescources())
	{
		if(getHarvesterTile())
		{
			HarvestTile = getHarvesterTile()->getNextHarvestTile();
			if(!HarvestTile)
			{
				DebugWarning("No tiles left. Return home and get killed");

				//If the tile is STILL null that mean that the harvester has no tiles anymore. Move back and destroy the worker
				GiveReturnJob();
				return;
			}else
			{
				//Mark the tile as taken
				HarvestTile->GiveHarvester();
			}
		}
	}
	
	if (!GetWorldTimerManager().IsTimerActive(HarvestTimer))
	{
		DebugWarning("Start Harvesting");

		if(getHarvesterTile())
		{
			float HarvestTime = FMath::RandRange(getHarvesterTile()->HarvestTime / 2.0f, getHarvesterTile()->HarvestTime);
			GetWorldTimerManager().SetTimer(HarvestTimer, this, &AHarvesterCreature::OnHarvest, HarvestTime, false);
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
