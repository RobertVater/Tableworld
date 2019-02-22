// Copyright by Robert Vater (Gunschlinger)

#include "HumanCreature.h"
#include "Player/TablePlayerController.h"

void AHumanCreature::OnPlayHarvestEffect()
{

}

void AHumanCreature::Create()
{
	Super::Create();

	//Set the name
	if(getGamemode())
	{
		if(getGamemode()->getGameInstance())
		{
			NameIndex = getGamemode()->getGameInstance()->getRandomHumanName(isMale());
		}
	}

	//Set the skintone

	//Default Skincolor is white
	SkinTone = 1.0f;

	if(FMath::RandRange(0.0f, 1.0f) <= .25f)
	{
		SkinTone = FMath::RandRange(0.0f, 0.25f);
		if(getDynMaterial())
		{
			getDynMaterial()->SetScalarParameterValue("SkinTone", getSkinTone());
		}
	}

}

void AHumanCreature::OnDeactivate()
{
	ATablePlayerController* PC = Cast<ATablePlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		PC->OnHumanDeactivated(this);
	}
}

FName AHumanCreature::getCreatureName()
{
	if (getGamemode())
	{
		if (getGamemode()->getGameInstance())
		{
			return getGamemode()->getGameInstance()->getHumanName(NameIndex, isMale());
		}
	}

	return FName();
}

float AHumanCreature::getSkinTone()
{
	return SkinTone;
}

UMaterialInstanceDynamic* AHumanCreature::getDynMaterial()
{
	if(!DynMaterial)
	{
		DynMaterial = Mesh->CreateDynamicMaterialInstance(0);
	}

	return DynMaterial;
}
