// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Misc/TableHelper.h"
#include "MainHud.generated.h"

class ATableGamemode;

UCLASS()
class TABLEWORLD_API UMainHud : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Notification")
	void OnNewNotification(FTableNotification  NewNotify);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Minimap")
	void UpdateMinimap(UTexture2D* NewMinimap);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Minimap")
	void MoveMinimapPlayerView(float AlphaX, float AlphaY, float ZoomAlpha, float ViewRectScale, float WorldScale);

	UFUNCTION(BlueprintCallable,BlueprintNativeEvent,Category = "Building")
	void BuildBuildableTiles();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Building")
	void UpdateRescourceUI();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Building")
	void UpdateGameTime(int32 NewGameTime);

	UFUNCTION(BlueprintCallable, Category = "Building")
	void SelectTool(EToolbarTools NewTool);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Toolbar")
	void ShowBuildMenu(bool bShow);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	EToolbarTools getSelectedTool();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	ATableGamemode* getGamemode();

protected:

	ATableGamemode* GM = nullptr;

	EToolbarTools SelectedTool = EToolbarTools::None;
};
