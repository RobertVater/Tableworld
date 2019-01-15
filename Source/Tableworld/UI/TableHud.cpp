// Copyright by Robert Vater (Gunschlinger)

#include "TableHud.h"
#include "Core/TableGameInstance.h"
#include "Core/TableGamemode.h"
#include "Kismet/GameplayStatics.h"

void ATableHud::BeginPlay()
{
	Super::BeginPlay();

	if(getGamemode())
	{
		getGamemode()->Event_FloatingItem.AddDynamic(this, &ATableHud::AddFloatingItem);
	}
}

void ATableHud::DrawHUD()
{
	Super::DrawHUD();

	float Delta = GetWorld()->GetDeltaSeconds();
	APlayerController* Con = GetWorld()->GetFirstPlayerController();

	if (Con) 
	{
		//Floating Icons
		for (int32 i = 0; i < FloatingItems.Num(); i++)
		{
			FFloatingItem Item = FloatingItems[i];
			if (Item.Alpha <= 0)
			{
				//Remove the icon
				FloatingItems.RemoveAt(i);
				continue;
			}

			Item.Alpha -= 1 * Delta;
			Item.YOffset -= 10 * Delta;

			FloatingItems[i] = Item;

			//Draw the Item
			FVector2D ScreenLoc;
			Con->ProjectWorldLocationToScreen(Item.WorldLoc, ScreenLoc, false);

			if(Item.Icon != nullptr)
			{
				float Scale = 1.5f;
				float IconSize = 32 * Scale;

				float X = (ScreenLoc.X - (IconSize / Scale) / 2);
				float Y = (ScreenLoc.Y - (IconSize / Scale)) + Item.YOffset;

				DrawText("+" + FString::FromInt(Item.Amount), FLinearColor::White, X-(IconSize / 2), Y + (IconSize / 4), NULL, Scale, false);
				DrawTexture(Item.Icon, X, Y, IconSize /  Scale, IconSize / Scale, IconSize / Scale, IconSize / Scale, 1.0f, 1.0f, FLinearColor::White, EBlendMode::BLEND_Translucent, Scale, false);
			}
		}
	}
}

void ATableHud::AddFloatingItem(EItem item, int32 Amount, FVector WorldLoc)
{
	if(getGameInstance())
	{
		FTableItem Item = getGameInstance()->getItem(item);

		if (Item.ItemIcon != nullptr) 
		{
			FFloatingItem FloatItem;
			FloatItem.Icon = Item.ItemIcon;
			FloatItem.Amount = Amount;

			FloatItem.WorldLoc = WorldLoc;

			FloatingItems.Add(FloatItem);
		}
	}
}

ATableGamemode* ATableHud::getGamemode()
{
	if(!GM)
	{
		GM = Cast<ATableGamemode>(UGameplayStatics::GetGameMode(this));
	}

	return GM;
}

UTableGameInstance* ATableHud::getGameInstance()
{
	if(!GI)
	{
		GI = Cast<UTableGameInstance>(UGameplayStatics::GetGameInstance(this));
	}

	return GI;
}
