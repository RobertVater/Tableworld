// Copyright by Robert Vater (Gunschlinger)

#include "TableCivilization.h"

void UTableCivilization::Create()
{
	LeftOverPoints = 10;
	TakenTraits.Empty();
}

bool UTableCivilization::BuyTrait(FCivTrait Data)
{
	int32 FoundIndex = 0;
	if (!HasTrait(Data, FoundIndex)) 
	{
		if(Data.bAddPoints)
		{
			//Add some Points
			LeftOverPoints += Data.TraitCost;
		}else
		{
			//Check if we can buy a trait
			if(LeftOverPoints >= Data.TraitCost)
			{
				LeftOverPoints -= Data.TraitCost;

			}else
			{
				return false;
			}
		}

		TakenTraits.Add(Data.ID);
		return true;
	}

	return false;
}

bool UTableCivilization::RemoveTrait(FCivTrait Data)
{
	int32 FoundIndex = 0;
	if (HasTrait(Data, FoundIndex))
	{
		if(Data.bAddPoints)
		{
			//Check if we can remove this trait
			if(LeftOverPoints >= Data.TraitCost)
			{
				LeftOverPoints -= Data.TraitCost;
			}else
			{
				return false;
			}
		}else
		{
			//Add Points
			LeftOverPoints += Data.TraitCost;
		}
		
		TakenTraits.RemoveAt(FoundIndex);
		return true;
	}

	return false;
}

bool UTableCivilization::HasTrait(FCivTrait Data, int32& FoundIndex)
{
	for(int32 i = 0; i < TakenTraits.Num(); i++)
	{
		FString ID = TakenTraits[i];
		if(ID == Data.ID)
		{
			FoundIndex = i;
			return true;
		}
	}

	return false;
}
