// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Misc/TableHelper.h"
#include "TablePlayerController.generated.h"

class UHudManager;
class ABuildableTile;
class AHumanCreature;

class ATableGamemode;
class ATableWorldTable;

UCLASS()
class TABLEWORLD_API ATablePlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	ATablePlayerController();
	virtual void BeginPlay() override;
	void InitController();

	void SelectTool(EToolbarTools NewTool);

	void ShowInfoPanel(FTableInfoPanel PanelData);

	void OnHumanDeactivated(AActor* Human);

	void UpdateMinimap();
	void UpdateMinimapPlayerView(float AlphaX, float AlphaY, float ZoomAlpha, float ViewScale, float WorldScale);

	UFUNCTION(BlueprintCallable,BlueprintNativeEvent, Category = "Getter")
	UHudManager* getHudManager();

	UFUNCTION(BlueprintCallable, Category = "Getter")
	ATableGamemode* getGamemode();

	UFUNCTION(BlueprintCallable, Category = "Getter")
	ATableWorldTable* getTable();

protected:

	ATableWorldTable* Table = nullptr;
	ATableGamemode* GM = nullptr;
};
