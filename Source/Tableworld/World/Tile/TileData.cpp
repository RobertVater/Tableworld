// Fill out your copyright notice in the Description page of Project Settings.

#include "TileData.h"
#include "../TableWorldTable.h"
#include "Tableworld.h"
#include "Misc/TableHelper.h"
#include "../TableChunk.h"
#include "DrawDebugHelpers.h"
#include "Building/BuildableTile.h"
#include "Core/TableGameInstance.h"

void UTileData::Set(int32 nX, int32 nY, int32 nLocalX, int32 nLocalY, ATableChunk* nTable)
{
	X = nX;
	Y = nY;

	LocalX = nLocalX;
	LocalY = nLocalY;

	WorldX = X * 100;
	WorldY = Y * 100;

	SetHeigth(getBaseHeigth());

	ParentChunk = nTable;
}

void UTileData::SetHeigth(float nHeight)
{
	Heigth = nHeight;
}

void UTileData::CopyTileData(UTileData* CopyTile)
{
	if(CopyTile)
	{
		PreviousTileType = CopyTile->getTileType();
		
		X = CopyTile->getX();
		Y = CopyTile->getY();

		LocalX = CopyTile->getLocalX();
		LocalY = CopyTile->getLocalY();

		WorldX = CopyTile->getWorldX();
		WorldY = CopyTile->getWorldY();

		ParentChunk = CopyTile->getParentChunk();

		TileObject = CopyTile->TileObject;

		SetHeigth(getBaseHeigth());

		TileRescource = CopyTile->getTileRescources();
		RescourceCount = CopyTile->getTileRescourceAmount();
	}
}

void UTileData::SetModified()
{
	bWasModified = true;
}

void UTileData::AddBuildableTile(ABuildableTile* nTileObject)
{
	TileObject = nTileObject;
}

void UTileData::ClearBuildingTile()
{
	TileObject = nullptr;
}

void UTileData::SetRescource(ETileRescources Type, int32 Amount, bool bUnlimited)
{
	TileRescource = Type;
	RescourceCount = Amount;
	bUnlimitedRescource = bUnlimited;

	LastResource = Type;
}

void UTileData::SetRescourceIndex(int32 Index)
{
	RescourceIndex = Index;
	LastIndex = Index;
}

void UTileData::GiveHarvester()
{
	bHasHarvester = true;

	bWasModified = true;
}

void UTileData::ClearHarvester()
{
	bHasHarvester = false;

	bWasModified = true;
}

void UTileData::UpdateRescource(int32 Amount)
{
	if(TileRescource != ETileRescources::None)
	{
		RescourceCount = Amount;

		bWasModified = true;
	}
}

void UTileData::ClearRescource()
{
	RescourceIndex = 0;
	RescourceCount = 0;
	TileRescource = ETileRescources::None;

	bHasHarvester = false;

	bWasModified = true;
}

void UTileData::SetGrassIDs(uint8 GrassIDs)
{
	GrassID = GrassIDs;
}

void UTileData::LowerGrass()
{
	if (ParentChunk)
	{
		if (ParentChunk->getTable()) 
		{
			FTransform Trans = ParentChunk->getGrassTransform(GrassID);

			FVector Loc = Trans.GetLocation();
			Loc.Z -= 200;

			Trans.SetLocation(Loc);
			ParentChunk->UpdateGrassTransform(GrassID, Trans);
		}
	}
}

void UTileData::DebugHighlightTile(float Time /*= 10.0f*/, FColor Color)
{
	if (UTableHelper::isDebug()) 
	{
		DrawDebugBox(ParentChunk->GetWorld(), getWorldCenter(), FVector(50, 50, 50), Color, false, Time, 0, 5.0f);
	}
}

int32 UTileData::getTileRescourceIndex()
{
	return RescourceIndex;
}

ETileRescources UTileData::getTileRescources()
{
	return TileRescource;
}

int32 UTileData::getTileRescourceAmount()
{
	if(bUnlimitedRescource)
	{
		return 1;
	}
	
	return RescourceCount;
}

FTransform UTileData::getRescourceTransform()
{
	if(ParentChunk)
	{
		return ParentChunk->getRescourceTransform(getTileRescources(), getTileRescourceIndex());
	}

	return FTransform();
}

ETileType UTileData::getTileType()
{
	return ETileType::Grass;
}

ETileType UTileData::getPreviousTileType()
{
	return PreviousTileType;
}

float UTileData::getBaseHeigth()
{
	return 0.0f;
}

float UTileData::getHeigth()
{
	return Heigth;
}

int32 UTileData::getX()
{
	return X;
}

int32 UTileData::getY()
{
	return Y;
}

FVector2D UTileData::getPositionAsVector()
{
	return FVector2D(getX(), getY());
}

int32 UTileData::getLocalX()
{
	return LocalX;
}

int32 UTileData::getLocalY()
{
	return LocalY;
}

int32 UTileData::getWorldX()
{
	return WorldX;
}

int32 UTileData::getWorldY()
{
	return WorldY;
}

FVector UTileData::getWorldCenter()
{
	return FVector(getWorldX() + 50, getWorldY() + 50, 0);
}

ATableChunk* UTileData::getParentChunk()
{
	return ParentChunk;
}

int32 UTileData::getFCost()
{
	return getGCost() + getHCost();
}

int32 UTileData::getHCost()
{
	return HCost;
}

int32 UTileData::getGCost()
{
	return GCost;
}

bool UTileData::IsBlocked(bool bIgnoreRescource)
{
	return !CanBuildOnTile(bIgnoreRescource);
}

int32 UTileData::getMovementCost()
{
	return 0;
}

bool UTileData::CanBuildOnTile(bool bIgnoreRescource)
{
	if (TileObject != nullptr)return false;
	if (!bIgnoreRescource && getTileRescources() != ETileRescources::None)return false;
	if (bIgnoreRescource && HasHarvester())return false;

	return true;
}

bool UTileData::HasRescource()
{
	return (getTileRescources() != ETileRescources::None);
}

bool UTileData::HasHarvester()
{
	return bHasHarvester;
}

bool UTileData::HasTileObject()
{
	return TileObject;
}

ABuildableTile* UTileData::getTileObject()
{
	return TileObject;
}

FText UTileData::getTileName()
{
	return FText::FromString("Tile");
}

bool UTileData::isModified()
{
	return bWasModified;
}

bool UTileData::HadRescource()
{
	return (LastResource != ETileRescources::None);
}

int32 UTileData::getLastRescourceIndex()
{
	return LastIndex;
}

FTableInfoPanel UTileData::getInfoPanelData_Implementation()
{
	FTableInfoPanel Data;

	UTableGameInstance* GI = Cast<UTableGameInstance>(getParentChunk()->GetGameInstance());
	if (GI)
	{
		bool bFound = false;
		FTableRescource Rescource = GI->getRescource(getTileRescources(), bFound);
		if (bFound)
		{
			FTableInfo_Text NameText;
			NameText.Icon = Rescource.RescourceIcon;
			NameText.RawText = Rescource.RescourceName;
			NameText.Size = 25;
			Data.DetailText.Add(NameText);

			FTableInfo_Text AmountText;

			if (!bUnlimitedRescource)
			{
				AmountText.RawText = FText::FromString("Amount: " + FString::FromInt(getTileRescourceAmount()));
			}
			else
			{
				AmountText.RawText = FText::FromString("Amount: Unlimited");
			}

			AmountText.Size = 20;
			Data.InfoText.Add(AmountText);
		}
	}

	Data.PanelSize = FVector2D(250, 150);
	Data.StaticWorldLocation = getWorldCenter();

	Data.WorldContext = this;
	Data.bOpenPanel = true;
	return Data;
}

FTableInfoPanel UTileData::getUpdateInfoPanelData_Implementation()
{
	FTableInfoPanel Data;

	if(!bUnlimitedRescource)
	{
		FTableInfo_Text AmountText;
		AmountText.RawText = FText::FromString("Amount: " + FString::FromInt(getTileRescourceAmount()));

		AmountText.Size = 20;
		Data.InfoText.Add(AmountText);
	}

	return Data;
}
