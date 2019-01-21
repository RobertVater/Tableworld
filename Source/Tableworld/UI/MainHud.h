// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Misc/TableHelper.h"
#include "MainHud.generated.h"

UCLASS()
class TABLEWORLD_API UMainHud : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Minimap")
	void UpdateMinimap(UTexture2D* NewMinimap);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Minimap")
	void MoveMinimapPlayerView(float AlphaX, float AlphaY, float ZoomAlpha);

	UFUNCTION(BlueprintCallable,BlueprintNativeEvent,Category = "Building")
	void BuildBuildableTiles();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Building")
	void UpdateRescourceUI();

	UFUNCTION(BlueprintCallable, Category = "Building")
	void SelectTool(EToolbarTools NewTool);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Building")
	EToolbarTools getSelectedTool();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Toolbar")
	void ShowBuildMenu(bool bShow);

protected:

	EToolbarTools SelectedTool = EToolbarTools::None;
};
