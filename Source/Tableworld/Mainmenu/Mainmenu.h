// Copyright by Robert Vater (Gunschlinger)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Mainmenu.generated.h"

UCLASS()
class TABLEWORLD_API AMainmenu : public AGameMode
{
	GENERATED_BODY()
	
public:

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Main")
	void NewGame();

	UFUNCTION(BlueprintCallable, Category = "Main")
	void LoadGame(FString SaveGame);

	UFUNCTION(BlueprintCallable, Category = "Main")
	bool DeleteGame(FString SaveGame);

	UFUNCTION(BlueprintCallable,Category = "Main")
	void LeaveMainmenu();

	UFUNCTION(BlueprintCallable, Category = "Main")
	TArray<FLoadedSavegame> ListSaveGames();

	UFUNCTION(BlueprintCallable, Category = "Main")
	void SetSeed(int32 nUsedSeed);

	UFUNCTION(BlueprintCallable, Category = "Main")
	void SetWorldSize(uint8 nWorldSize);

	UFUNCTION(BlueprintCallable, Category = "Main")
	void SetRivers(bool bnGenRiver, uint8 nRiverCount);

	UFUNCTION(BlueprintCallable, Category = "Main")
	UTexture2D* GenerateMapPreview();

protected:

	int32 UsedSeed = 0;
	int32 WorldSize = 8;
	bool bGenerateRivers = true;
	uint8 RiverCount = 4;

	UPROPERTY()
	UTexture2D* MapPreview = nullptr;
};
