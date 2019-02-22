// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "InfoPanelInterface.generated.h"

UINTERFACE()
class TABLEWORLD_API UInfoPanelInterface : public UInterface
{
	GENERATED_BODY()
};

class IInfoPanelInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "InfoPanel")
	FTableInfoPanel getInfoPanelData();
};
