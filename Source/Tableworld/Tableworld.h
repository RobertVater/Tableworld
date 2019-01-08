// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#define SLOMO_CUR_LINE  (FString::FromInt(__LINE__))
#define SLOMO_CUR_CLASS (FString(__FUNCTION__).Right(FString(__FUNCTION__).Len() - FString(__FUNCTION__).Find(TEXT("::")) - 2 ))
#define SLOMO_CUR_CLASS_LINE (SLOMO_CUR_CLASS + ": " + SLOMO_CUR_LINE)

#define DebugError(text) UE_LOG(LogTemp,Error,TEXT("[%s] %s"),*SLOMO_CUR_CLASS_LINE, *FString(text))
#define DebugWarning(text) UE_LOG(LogTemp,Warning,TEXT("[%s] %s"),*SLOMO_CUR_CLASS_LINE, *FString(text))
#define DebugLog(text) UE_LOG(LogTemp,Log,TEXT("[%s] %s"),*SLOMO_CUR_CLASS_LINE, *FString(text))