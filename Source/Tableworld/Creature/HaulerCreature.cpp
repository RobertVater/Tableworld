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

	SetCreatureStatus(ECreatureStatus::GoingToWork);
	RoadMoveTo(nTargetTile);
}

void AHaulerCreature::GiveProductionHaulJob(ACityCentreTile* nInventory, AProductionBuilding* nHomeProductionBuilding, UTileData* nTargetTile, UTileData* nHomeTile)
{
	Inventory = nInventory;
	HomeProductionBuilding = nHomeProductionBuilding;

	HaulTile = nTargetTile;
	HomeTile = nHomeTile->getPositionAsVector();

	SetCreatureStatus(ECreatureStatus::GoingToWork);
	RoadMoveTo(nTargetTile);
}

void AHaulerCreature::GiveReturnJob()
{
	if(getGamemode())
	{
		if (HasItems())
		{
			SetCreatureStatus(ECreatureStatus::ReturningGoods);
		}else
		{
			SetCreatureStatus(ECreatureStatus::GoingToWork);
		}

		//Reverse the last calculated path.
		RetracePath();
	}
}

void AHaulerCreature::ForceReturnJob()
{
	if (getGamemode())
	{
		if (HasItems())
		{
			SetCreatureStatus(ECreatureStatus::ReturningGoods);
		}
		else
		{
			SetCreatureStatus(ECreatureStatus::GoingToWork);
		}

		//Reverse the last calculated path.
		Algo::Reverse(PathPoints);

		//Remove every point until we found our points
		UTileData* OurTile = getStandingTile();
		if(OurTile)
		{
			for(int32 i = 0; i < PathPoints.Num(); i++)
			{
				FVector Loc = PathPoints[i];
				if(Loc.Equals(OurTile->getWorldCenter()) )
				{
					break;
				}

				PathPoints.RemoveAt(i);
			}
		}
		
		CurrentPathIndex = 0;
	}
}

void AHaulerCreature::AddHaulItems(EItem nItem, int32 nAmount)
{
	CarriedItems.Add(nItem, nAmount);

	DebugWarning("Add Haul Items! " + FString::FromInt(nAmount));
}

void AHaulerCreature::OnMoveCompleted()
{
	Super::OnMoveCompleted();
	
	UTileData* OurTile = getStandingTile();
	if(OurTile)
	{
		if (!bHauledItems) 
		{
			if (HaulTile)
			{
				if (HaulTile->getX() == OurTile->getX())
				{
					if (HaulTile->getY() == OurTile->getY())
					{
						DebugWarning("Reached Target");

						//We reached the target building
						Event_HaulerReachedTarget.Broadcast(this);

						if (TargetInventory) 
						{
							TargetInventory->TransferInventory(this);
							GiveReturnJob();
						}

						bHauledItems = true;
						return;
					}
				}
			}
		}

		if (HomeTile.X == OurTile->getX())
		{
			if (HomeTile.Y == OurTile->getY())
			{
				DebugWarning("Reached Home");

				//We reached our home base
				Event_HaulerReturnedHome.Broadcast(this);
				if (HomeCityCentre)
				{
					HomeCityCentre->OnHaulCompleted(this);
				}
				return;
			}
		}
	}
}

bool AHaulerCreature::getItemZero(EItem& Item, int32& Amount)
{
	for(auto Elem : getCarriedItems())
	{
		Item = Elem.Key;
		Amount = Elem.Value;

		return true;
	}

	return false;
}

TMap<EItem, int32> AHaulerCreature::getCarriedItems()
{
	return CarriedItems;
}

bool AHaulerCreature::HasItems()
{
	return (getCarriedItems().Num() > 0);
}

ACityCentreTile* AHaulerCreature::getInventoryBuilding()
{
	return Inventory;
}

AProductionBuilding* AHaulerCreature::getHomeProductionBuilding()
{
	return HomeProductionBuilding;
}

UAnimationAsset* AHaulerCreature::getWalkAnimation()
{
	if(getCarriedItems().Num() > 0)
	{
		return WalkLoaded;
	}

	return Walk;
}
