// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TableHud.generated.h"

class ATableGamemode;
class UTableGameInstance;

struct FFloatingItem
{
	UTexture2D* Icon = nullptr;
	int32 Amount = 1;

	FVector WorldLoc;

	float YOffset = 0.0f;
	float Alpha = 1.0f;
};

UCLASS()
class TABLEWORLD_API ATableHud : public AHUD
{
	GENERATED_BODY()
	
public:

	virtual void BeginPlay() override;
	virtual void DrawHUD() override;

	UFUNCTION()
	void AddFloatingItem(EItem item, int32 Amount, FVector WorldLoc);

	ATableGamemode* getGamemode();
	UTableGameInstance* getGameInstance();

protected:

	//A Array containing all active floating icons in the world
	TArray<FFloatingItem> FloatingItems;

	ATableGamemode* GM = nullptr;
	UTableGameInstance* GI = nullptr;
};
