// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialogChoice.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDialogChoice_OptionA);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDialogChoice_OptionB);

UCLASS()
class TABLEWORLD_API UDialogChoice : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Events")
	void Set(const FText& Title, const FText& Text, const FText& A, const FText& B);

	UFUNCTION(BlueprintCallable,Category = "Events")
	void OptionA();

	UFUNCTION(BlueprintCallable, Category = "Events")
	void OptionB();

	UPROPERTY()
	FDialogChoice_OptionA Event_OptionA;

	UPROPERTY()
	FDialogChoice_OptionB Event_OptionB;
};
