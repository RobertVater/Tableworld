// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PopupInfo.generated.h"

/**
 * 
 */
UCLASS()
class TABLEWORLD_API UPopupInfo : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable,BlueprintNativeEvent, Category = "Setter")
	void Set(const FText& Title, const FText& Text);
};
