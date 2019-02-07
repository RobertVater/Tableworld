// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Mainmenu.generated.h"

/**
 * 
 */
UCLASS()
class TABLEWORLD_API AMainmenu : public AGameMode
{
	GENERATED_BODY()
	
public:

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable,Category = "Main")
	void LeaveMainmenu();

	UFUNCTION(BlueprintCallable, Category = "Main")
	void SetSeed(int32 nUsedSeed);

	UFUNCTION(BlueprintCallable, Category = "Main")
	void SetWorldSize(uint8 nWorldX, uint8 nWorldY);

	UFUNCTION(BlueprintCallable, Category = "Main")
	UTexture2D* GenerateMapPreview();

protected:

	int32 UsedSeed = 0;
	int32 WorldX = 8;
	int32 WorldY = 8;

	UPROPERTY()
	UTexture2D* MapPreview = nullptr;
};
