// Copyright by Robert Vater (Gunschlinger)

#include "HaulerCreature.h"
#include "World/Tile/Building/InventoryTile.h"
#include "World/Tile/TileData.h"
#include "World/Tile/Building/CityCentreTile.h"
#include "Core/TableGamemode.h"
#include "DrawDebugHelpers.h"
#include "World/TableWorldTable.h"

void AHaulerCreature::GiveHaulJob(FName nTargetBuilding, FName nHomeBuilding, UTileData* nTargetTile, UTileData* nHomeTile)
{
	HaulTile = nTargetTile;
	
	HomeBuildingUID = nHomeBuilding;
	TargetBuildingUID = nTargetBuilding;
	
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
		TArray<FVector> NewPath;

		//Remove every point until we found our point
		for (int32 i = 0; i < PathPoints.Num(); i++)
		{
			if(i >= CurrentPathIndex)
			{
				break;
			}
			
			NewPath.Add(PathPoints[i]);
		}

		//Reverse the last calculated path.
		Algo::Reverse(NewPath);
		PathPoints = NewPath;
		
		CurrentPathIndex = 0;
	}
}

void AHaulerCreature::AddHaulItems(EItem nItem, int32 nAmount)
{
	CarriedItems.Add(nItem, nAmount);
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
				DebugError("HaulTile");

				if (HaulTile->getX() == OurTile->getX())
				{
					if (HaulTile->getY() == OurTile->getY())
					{
						DebugError("0");
						
						//We reached the target building
						Event_HaulerReachedTarget.Broadcast(this);

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
				//We reached our home base
				Event_HaulerReturnedHome.Broadcast(this);
				return;
			}
		}
	}
}

void AHaulerCreature::ClearInventory()
{
	bHauledItems = false;
	CarriedItems.Empty();
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

FName AHaulerCreature::getHomeBuildingUID()
{
	return HomeBuildingUID;
}

FName AHaulerCreature::getTargetBuildingUID()
{
	return TargetBuildingUID;
}

UAnimationAsset* AHaulerCreature::getWalkAnimation()
{
	if(getCarriedItems().Num() > 0)
	{
		return WalkLoaded;
	}

	return Walk;
}

void AHaulerCreature::LoadData(FTableSaveHaulerCreature Data)
{
	//BaseCreature
	HomeTile = Data.HomeTile;
	CurrentPathIndex = Data.PathIndex;
	PathPoints = Data.Path;
	LastCalculatedPath = UTableHelper::Convert2DVectorArrayToTileArray(Data.LastCalcPath);
	MinDistance = Data.MinDinstance;
	SetCreatureStatus(Data.Status);
	UpdateCreatureStatus();

	//Hauler
	if(getGamemode())
	{
		ATableWorldTable* Table = getGamemode()->getTable();
		if(Table)
		{
			//Haultile
			if(Data.HaulTile > FVector2D(-1,-1))
			{
				HaulTile = getGamemode()->getTile((int32)Data.HaulTile.X, (int32)Data.HaulTile.Y);
				HaulTile->DebugHighlightTile(10);
			}
		}
	}

	TargetBuildingUID = Data.TargetBuildingUID;
	HomeBuildingUID = Data.HomeBuildingUID;

	bHauledItems = Data.bHauledItems;
	CarriedItems = Data.CarriedItems;
}

FTableSaveHaulerCreature AHaulerCreature::getSaveData()
{
	FTableSaveHaulerCreature Data;
	
	//BaseCreature
	Data.HomeTile = HomeTile;
	Data.MinDinstance = MinDistance;
	Data.PathIndex = CurrentPathIndex;
	Data.Path = PathPoints;
	Data.LastCalcPath = UTableHelper::ConvertTileArrayTo2DVectorArray(LastCalculatedPath);
	Data.Location = GetActorLocation();
	Data.Status = getStatus();

	//Hauler
	Data.bHauledItems = bHauledItems;
	Data.CarriedItems = CarriedItems;

	//Hauler data
	if (HaulTile) 
	{
		Data.HaulTile = HaulTile->getPositionAsVector();
	}

	Data.HomeBuildingUID = HomeBuildingUID;
	Data.TargetBuildingUID = TargetBuildingUID;

	return Data;
}
