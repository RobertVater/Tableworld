// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ObjectAndNameAsStringProxyArchive.h"
#include "TableSavegame.generated.h"

struct FTableSaveGameArchive : public FObjectAndNameAsStringProxyArchive
{
	FTableSaveGameArchive(FArchive& InInnerArchive) : FObjectAndNameAsStringProxyArchive(InInnerArchive, true)
	{
		ArIsSaveGame = true;
		ArNoDelta = true; // Optional, useful when saving/loading variables without resetting the level.
						  // Serialize variables even if weren't modified and mantain their default values.
	}
};

USTRUCT(BlueprintType)
struct FObjectRecord
{

	GENERATED_USTRUCT_BODY()

public:

	// class that this object is
	UPROPERTY(BlueprintReadWrite)
	UClass* Class;

	// save the outer used for object so they get loaded back in the correct hierachy
	UPROPERTY(BlueprintReadWrite)
	UObject* Outer;

	// save the outer used for object so they get loaded back in the correct hierachy
	UPROPERTY(BlueprintReadWrite)
	int32 OuterID;

	// if the outer is an actor otherwise will be UObject
	UPROPERTY(BlueprintReadWrite)
	bool bActor;

	// this is for loading only, store a pointer for the loaded object here so you can loop for the records later to de-serialize all the data
	UPROPERTY(BlueprintReadWrite)
	UObject* Self;

	// Name of the object
	UPROPERTY(BlueprintReadWrite)
	FName Name;

	// serialized data for all UProperties that are 'SaveGame' enabled
	UPROPERTY(BlueprintReadWrite)
	TArray<uint8> Data;

	// Spawn location if it's an actor
	UPROPERTY(BlueprintReadWrite)
	FTransform Transform;

	FObjectRecord()
	{
		Class = nullptr;
		Outer = nullptr;
		Self = nullptr;
	}
};

UCLASS()
class TABLEWORLD_API UTableSavegame : public USaveGame
{
	GENERATED_BODY()

public:


};
