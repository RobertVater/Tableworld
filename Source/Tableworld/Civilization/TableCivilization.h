// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Misc/TableHelper.h"
#include "TableCivilization.generated.h"

/**
 * 
 */
UCLASS()
class TABLEWORLD_API UTableCivilization : public UObject
{
	GENERATED_BODY()
	
public:

	FText CivName;
	FText CivTitle;

	int32 LeftOverPoints = 0;
	TArray<FString> TakenTraits;

	void Create();

	bool BuyTrait(FCivTrait Data);
	bool RemoveTrait(FCivTrait Data);

	bool HasTrait(FCivTrait Data, int32& FoundIndex);
};
