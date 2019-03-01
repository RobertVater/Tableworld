// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "TableCivSavegame.generated.h"

/**
 * 
 */
UCLASS()
class TABLEWORLD_API UTableCivSavegame : public USaveGame
{
	GENERATED_BODY()

public:
	
	UPROPERTY()
	FName Name;

	UPROPERTY()
	FName Title;

	UPROPERTY()
	TArray<FString> TakenTraits;

	UPROPERTY()
	FName Version = "1.0";
};
