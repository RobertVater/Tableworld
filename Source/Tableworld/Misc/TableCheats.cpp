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
