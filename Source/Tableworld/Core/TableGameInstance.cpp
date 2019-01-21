// Fill out your copyright notice in the Description page of Project Settings.

#include "TableGameInstance.h"
#include "Misc/TableHelper.h"

FTableBuilding UTableGameInstance::getBuilding_Implementation(const FName& ID, bool& bFound)
{
	return FTableBuilding();
}

TArray<FTableBuilding> UTableGameInstance::getBuildingsForAgeAndCategory_Implementation(ETableAge Age, ETableTileCategory Category, bool& bFound)
{
	return TArray<FTableBuilding>();
}

FTableItem UTableGameInstance::getItem_Implementation(EItem Item, bool& bFound)
{
	return FTableItem();
}

FTableRescource UTableGameInstance::getRescource_Implementation(ETileRescources Rescource, bool& bFound)
{
	return FTableRescource();
}
