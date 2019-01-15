// Copyright by Robert Vater (Gunschlinger)

#include "HaulerCreature.h"
#include "World/Tile/Building/InventoryTile.h"
#include "World/Tile/TileData.h"
#include "World/Tile/Building/CityCentreTile.h"
#include "Core/TableGamemode.h"
#include "DrawDebugHelpers.h"

void AHaulerCreature::GiveHaulJob(AInventoryTile* nTargetInventory, ACityCentreTile* nHomeCityCentre, UTileData* nTargetTile, UTileData* nHomeTile)
{
	HaulTile = nTargetTile;
	HomeCityCentre = nHomeCityCentre;
	TargetInventory = nTargetInventory;
	
	HomeTile = nHomeTile->getPositionAsVector();

	RoadMoveTo(nTargetTile);
}

void AHaulerCreature::GiveReturnJob()
{
	if(getGamemode())
	{
		//Reverse the last calculated path.
		RetracePath();
	}
}

void AHaulerCreature::AddHaulItems(EItem nItem, int32 nAmount)
{
	HaulItem = nItem;
	HaulAmount = nAmount;

	DebugWarning("Add Haul Items! " + FString::FromInt(nAmount));
}

void AHaulerCreature::OnMoveCompleted()
{
	UTileData* OurTile = getStandingTile();
	if(OurTile)
	{
		if (!bHauledItems) 
		{
			if (HaulTile && TargetInventory)
			{
				if (HaulTile->getX() == OurTile->getX())
				{
					if (HaulTile->getY() == OurTile->getY())
					{
						DebugWarning("Reached Target");

						//We reached the target building
						TargetInventory->TransferInventory(this);
						GiveReturnJob();

						bHauledItems = true;
						return;
					}
				}
			}
		}

		if (HomeCityCentre) 
		{
			if (HomeTile.X == OurTile->getX())
			{
				if (HomeTile.Y == OurTile->getY())
				{
					DebugWarning("Reached Home");

					//We reached our home base
					HomeCityCentre->OnHaulCompleted(this);
					return;
				}
			}
		}
	}
}

EItem AHaulerCreature::getHaulItem()
{
	return HaulItem;
}

int32 AHaulerCreature::getHaulAmount()
{
	return HaulAmount;
}
