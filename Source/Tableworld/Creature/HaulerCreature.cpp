// Copyright by Robert Vater (Gunschlinger)

#include "HaulerCreature.h"
#include "World/Tile/Building/ProductionBuilding.h"
#include "World/Tile/TileData.h"
#include "World/Tile/Building/CityCentreBuilding.h"
#include "Core/TableGamemode.h"
#include "DrawDebugHelpers.h"
#include "World/TableWorldTable.h"
#include "Component/InventoryComponent.h"

AHaulerCreature::AHaulerCreature()
{
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
}

void AHaulerCreature::GiveHaulJob(FName nTargetBuilding, FName nHomeBuilding, UTileData* nHomeTile, UTileData* nTargetTile)
{
	HaulTile = nTargetTile;
	HomeTile = nHomeTile->getPositionAsVector();
	
	HomeBuildingUID = nHomeBuilding;
	TargetBuildingUID = nTargetBuilding;

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
	InventoryComponent->ModifyInventory(nItem, nAmount);
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
						DebugLog("ReachedTarget");
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
				DebugLog("ReturnedHome");

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
	InventoryComponent->ClearInventory();
}

TMap<EItem, int32> AHaulerCreature::getCarriedItems()
{
	return InventoryComponent->getInventory();
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

FName AHaulerCreature::getReadableStatus()
{
	if(getStatus() == ECreatureStatus::GoingToWork)
	{
		return "Collecting Items";
	}

	return Super::getReadableStatus();
}

UAnimationAsset* AHaulerCreature::getWalkAnimation()
{
	if(getCarriedItems().Num() > 0)
	{
		return WalkLoaded;
	}

	return Walk;
}

FTableInfoPanel AHaulerCreature::getInfoPanelData_Implementation()
{
	FTableInfoPanel Data = Super::getInfoPanelData_Implementation();

	Data.InventoryComponent = InventoryComponent;
	Data.PanelSize.Y += 50.0f;

	return Data;
}
