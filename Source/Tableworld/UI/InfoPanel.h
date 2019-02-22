// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InfoPanel.generated.h"

/**
 * 
 */
UCLASS()
class TABLEWORLD_API UInfoPanel : public UUserWidget
{
	GENERATED_BODY()
	
public:

	virtual void OpenPanel(FTableInfoPanel nPanelData);

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Setup")
	void SetupInfo(FTableInfoPanel PanelData);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	AActor* getWorldContextActor();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	virtual FVector getTargetWorldLocation();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter")
	virtual FTableInfoPanel getPanelData();

protected:

	FTableInfoPanel PanelData;

	AActor* TargetActor = nullptr;
	FVector StaticWorldLocation = FVector::ZeroVector;
	bool bHasStaticLocation = false;
};
