// Copyright by Robert Vater (Gunschlinger)

#include "TableCheats.h"
#include "Core/TableGamemode.h"
#include "Kismet/GameplayStatics.h"

void UTableCheats::Give(EItem ItemID /*= "Log"*/, int32 Amount /*= 1*/)
{
	ATableGamemode* GM = Cast<ATableGamemode>(UGameplayStatics::GetGameMode(GetWorld()->GetFirstPlayerController()));
	if(GM)
	{
		GM->ModifyRescource(ItemID, Amount);
	}
}

void UTableCheats::Starter()
{
	ATableGamemode* GM = Cast<ATableGamemode>(UGameplayStatics::GetGameMode(GetWorld()->GetFirstPlayerController()));
	if (GM)
	{
		GM->ModifyRescource(EItem::Stone, 999);
		GM->ModifyRescource(EItem::WoodLog, 999);
	}
}

void UTableCheats::SpawnFloater()
{
	ATableGamemode* GM = Cast<ATableGamemode>(UGameplayStatics::GetGameMode(GetWorld()->GetFirstPlayerController()));
	if (GM)
	{
		GM->AddFloatingItem(EItem::BronzeBar, FMath::RandRange(1, 999), GM->getPlayerPawn()->GetActorLocation());
	}
}
