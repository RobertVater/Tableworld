// Copyright by Robert Vater (Gunschlinger)

#include "DialogChoice.h"

void UDialogChoice::Set_Implementation(const FText& Title, const FText& Text, const FText& A, const FText& B)
{

}

void UDialogChoice::OptionA()
{
	Event_OptionA.Broadcast();

	RemoveFromParent();
}

void UDialogChoice::OptionB()
{
	Event_OptionB.Broadcast();

	RemoveFromParent();
}
