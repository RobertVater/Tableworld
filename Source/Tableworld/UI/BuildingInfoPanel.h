// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BuildingInfoPanel.generated.h"

class ABuildableTile;

UCLASS()
class TABLEWORLD_API UBuildingInfoPanel : public UUserWidget
{
	GENERATED_BODY()
	
public:

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void OpenBuildingInfo(ABuildableTile* nSelectedBuilding);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Setup")
	void SetupInfo(ABuildableTile* Building);

	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "Getter")
	ABuildableTile* getSelectedBuilding();

protected:

	ABuildableTile* SelectedBuilding = nullptr;
};
