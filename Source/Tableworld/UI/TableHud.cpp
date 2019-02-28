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
				float Scale = 64.0f / Item.Icon->GetSurfaceWidth();
				float IconSize = 32 * Scale;

				float ImageSize = Item.Icon->GetSurfaceWidth() * Scale;
				float ImageSizeHalf = ImageSize / 2;

				float X = (ScreenLoc.X - (IconSize / Scale) / 2);
				float Y = (ScreenLoc.Y - (IconSize / Scale)) + Item.YOffset;

				//BlackBackground
				DrawText("+" + FString::FromInt(Item.Amount), FLinearColor::White, X - ImageSize, Y + (ImageSizeHalf / 2.0f), NULL, 2.0f);
				DrawTextureSimple(Item.Icon, X, Y, Scale);
			}
		}
	}
}

void ATableHud::AddFloatingItem(EItem item, int32 Amount, FVector WorldLoc)
{
	if(getGameInstance())
	{
		bool bFoundItem = false;
		FTableItem Item = getGameInstance()->getItem(item,bFoundItem);

		if (bFoundItem) 
		{
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
