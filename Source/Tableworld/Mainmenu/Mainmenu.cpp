// Copyright by Robert Vater (Gunschlinger)

#include "Mainmenu.h"
#include "Misc/Math/FastNoise.h"
#include "World/TableWorldTable.h"
#include "World/MapGenerator.h"
#include "PlatformFilemanager.h"
#include "UI/DialogChoice.h"
#include "Civilization/TableCivilization.h"
#include "Savegame/TableCivSavegame.h"

void AMainmenu::BeginPlay()
{
	Super::BeginPlay();
}

void AMainmenu::SelectMenuState_Implementation(int32 ActiveID)
{

}

void AMainmenu::ShowInfoBox(const FText& Title, const FText& Text)
{
	UTableGameInstance* GI = Cast<UTableGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (GI)
	{
		GI->CreatePopupInfo(Title, Text);
	}
}

void AMainmenu::CivNew_LeftPoints_Accept()
{
	SaveNewCiv();
	FinalizeCiv();
}

UDialogChoice* AMainmenu::ShowDialogBox(FText Title, FText Text, FText OptionA, FText OptionB)
{
	UTableGameInstance* GI = Cast<UTableGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (GI)
	{
		DebugLog("Dialog");
		return GI->CreateDialogChoice(Title, Text, OptionA, OptionB);
	}

	return nullptr;
}

void AMainmenu::NewGame()
{
	UTableGameInstance* GI = Cast<UTableGameInstance>(UGameplayStatics::GetGameInstance(this));
	if(GI)
	{
		//Save the chosen settings
		GI->PrepareNewGame(UsedSeed, WorldSize, bGenerateRivers, RiverCount);
		
		//Leave the main menu
		LeaveMainmenu();

		//Load a new map
		UGameplayStatics::OpenLevel(this, "Gamemap");
	}
}

void AMainmenu::LoadGame(FString SaveGame)
{
	//Check if the savegame is valid
	if(!UGameplayStatics::DoesSaveGameExist(SaveGame,0))
	{
		DebugError("Save " + SaveGame + " does not exist");
		return;
	}
	
	UTableGameInstance* GI = Cast<UTableGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (GI)
	{
		//Preparing loading
		GI->PrepareLoadGame(SaveGame);

		//Leave the main menu
		LeaveMainmenu();

		//Load a new map
		UGameplayStatics::OpenLevel(this, "Gamemap");
	}
}

bool AMainmenu::DeleteGame(FString SaveGame)
{
	FString SavegamePath = FString(FPaths::GameSavedDir()) + "SaveGames/" + SaveGame + ".sav";

	if(!FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*SavegamePath))
	{
		DebugError("Failed to delete " + SaveGame + "!");
		return false;
	}

	return true;
}

void AMainmenu::LeaveMainmenu()
{
	if(MapPreview)
	{
		MapPreview->RemoveFromRoot();
		MapPreview = nullptr;
	}
}

TArray<FLoadedSavegame> AMainmenu::ListSaveGames()
{
	TArray<FLoadedSavegame> SaveGames;

	FString SavegamePath = FString(FPaths::GameSavedDir()) + "SaveGames/*.sav";
	DebugLog(SavegamePath);
	IFileManager& FileManager = IFileManager::Get();

	TArray<FString> Files;
	FileManager.FindFiles(Files, *SavegamePath, true, false);

	DebugLog("Found " + FString::FromInt(Files.Num()) + " Files!");

	for(FString FileString : Files)
	{
		DebugLog("-" + FileString);

		FLoadedSavegame NewSave;
		NewSave.FileName = FName(*FileString.Left(FileString.Len() - 4));

		SaveGames.Add(NewSave);
	}

	return SaveGames;
}

TArray<FLoadedCiv> AMainmenu::ListCivs()
{
	TArray<FLoadedCiv> Civs;

	FString SavegamePath = FString(FPaths::GameSavedDir()) + "SaveGames/Civilization/*.sav";
	DebugLog(SavegamePath);
	IFileManager& FileManager = IFileManager::Get();

	TArray<FString> Files;
	FileManager.FindFiles(Files, *SavegamePath, true, false);

	DebugLog("Found " + FString::FromInt(Files.Num()) + " Civ Files!");

	for (FString FileString : Files)
	{
		UTableCivSavegame* Save = Cast<UTableCivSavegame>(UGameplayStatics::LoadGameFromSlot("Civilization/" + FileString.Left(FileString.Len() - 4), 0));
		if(Save)
		{
			FLoadedCiv NewCiv;
			NewCiv.CivName = Save->Name.ToString();
			NewCiv.CivTitle = Save->Title.ToString();
			NewCiv.FileName = FileString.Left(FileString.Len() - 4);
			NewCiv.SaveVersion = Save->Version.ToString();

			NewCiv.TakenTraits = Save->TakenTraits;

			DebugLog("Loaded " + NewCiv.CivName);
			Civs.Add(NewCiv);
		}
	}

	return Civs;
}

void AMainmenu::SetSeed(int32 nUsedSeed)
{
	UsedSeed = nUsedSeed;
}

void AMainmenu::SetWorldSize(uint8 nWorldSize)
{
	WorldSize = nWorldSize;
}

void AMainmenu::SetRivers(bool bnGenRiver, uint8 nRiverCount)
{
	bGenerateRivers = bnGenRiver;
	RiverCount = nRiverCount;
}

UTexture2D* AMainmenu::GenerateMapPreview()
{
	//Apply the new Seed
	FMath::RandInit(UsedSeed);
	
	int32 TextureSize = WorldSize * 16;

	if(MapPreview)
	{
		//Null the old map
		MapPreview->RemoveFromRoot();
		MapPreview = nullptr;
	}

	//Create the MapPreview texture
	MapPreview = UTexture2D::CreateTransient(TextureSize, TextureSize);
	MapPreview->AddToRoot();
	MapPreview->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	MapPreview->SRGB = false;
	MapPreview->Filter = TextureFilter::TF_Nearest;
	MapPreview->UpdateResource();

	if (MapPreview) 
	{
		TArray<FGeneratedMapTile> Tiles = MapGenerator::GenerateMap(UsedSeed, WorldSize, bGenerateRivers, RiverCount);

		uint8* Pixels = new uint8[TextureSize * TextureSize * 4];
		for (int32 x = 0; x < TextureSize; x++)
		{
			for (int32 y = 0; y < TextureSize; y++)
			{
				FColor PixelColor = FColor::Green;

				int32 TileIndex = y * TextureSize + x;
				if(Tiles.IsValidIndex(TileIndex))
				{
					FGeneratedMapTile TileData = Tiles[TileIndex];

					PixelColor = MapGenerator::getTileColor(TileData.TileType, TileData.Resscource);
				}
				int32 PixelIndex = y * TextureSize + x;
				Pixels[4 * PixelIndex + 2] = PixelColor.R;
				Pixels[4 * PixelIndex + 1] = PixelColor.G;
				Pixels[4 * PixelIndex + 0] = PixelColor.B;
				Pixels[4 * PixelIndex + 3] = PixelColor.A;
			}
		}

		FTexture2DMipMap& Mip = MapPreview->PlatformData->Mips[0];
		Mip.BulkData.Lock(LOCK_READ_WRITE);

		uint8* TextureData = (uint8*)Mip.BulkData.Realloc(TextureSize*TextureSize * 4);
		FMemory::Memcpy(TextureData, Pixels, sizeof(uint8) * (TextureSize * TextureSize * 4));
		Mip.BulkData.Unlock();

		MapPreview->UpdateResource();

		delete[] Pixels;
	}

	return MapPreview;
}

void AMainmenu::CreateCiv()
{
	if(Civilization)
	{
		//Reset
		Civilization->Create();

		return;
	}

	Civilization = NewObject<UTableCivilization>(UTableCivilization::StaticClass());
	if(Civilization)
	{
		Civilization->Create();
	}
}

bool AMainmenu::SetCivDetails(FText Name, FText Title)
{
	if(Civilization)
	{
		if (Name.ToString().Len() <= 0) 
		{
			ShowInfoBox(FText::FromString("Error"), FText::FromString("You forgot to add a name!"));
			return false; 
		}

		if (Title.ToString().Len() <= 0)
		{
			ShowInfoBox(FText::FromString("Error"), FText::FromString("You forgot to add a title!"));
			return false;
		}

		Civilization->CivName = Name;
		Civilization->CivTitle = Title;

		if(Civilization->LeftOverPoints > 0)
		{
			UDialogChoice* ChoiceWidget = ShowDialogBox(FText::FromString("Info"), FText::FromString("You still have traitpoints left. Are you sure you want to continue ?"), FText::FromString("Yes"), FText::FromString("No"));
			if(ChoiceWidget)
			{
				ChoiceWidget->Event_OptionA.AddDynamic(this, &AMainmenu::CivNew_LeftPoints_Accept);

				return false;
			}
		}

		SaveNewCiv();
		FinalizeCiv();
		return true;
	}

	return false;
}

void AMainmenu::SaveNewCiv()
{
	if (Civilization) 
	{
		UTableCivSavegame* Civ = Cast<UTableCivSavegame>(UGameplayStatics::CreateSaveGameObject(UTableCivSavegame::StaticClass()));
		if (Civ)
		{
			Civ->Name = FName(*Civilization->CivName.ToString());
			Civ->Title = FName(*Civilization->CivTitle.ToString());
			Civ->TakenTraits = Civilization->TakenTraits;

			UGameplayStatics::SaveGameToSlot(Civ, "Civilization/" + Civ->Name.ToString(), 0);

			DebugLog(Civ->Name.ToString() + " saved!");
		}
	}
}

bool AMainmenu::DeleteCiv(FString SaveGame)
{
	FString SavegamePath = FString(FPaths::GameSavedDir()) + "SaveGames/Civilizations/" + SaveGame + ".sav";

	if (!FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*SavegamePath))
	{
		DebugError("Failed to delete " + SaveGame + "!");
		return false;
	}

	return true;
}

void AMainmenu::FinalizeCiv()
{
	if(Civilization)
	{
		//Copy the civ data into the gameinstance.
		UTableGameInstance* GI = Cast<UTableGameInstance>(UGameplayStatics::GetGameInstance(this));
		if (GI)
		{
			GI->SetCivilization(Civilization);
		}

		//Move into the worldgen gamestate
		SelectMenuState(3);
	}
}

bool AMainmenu::BuyTrait(FCivTrait Trait)
{
	if (Civilization) 
	{
		return Civilization->BuyTrait(Trait);
	}

	return false;
}

bool AMainmenu::RefundTrait(FCivTrait Trait)
{
	if(Civilization)
	{
		return Civilization->RemoveTrait(Trait);
	}

	return false;
}

int32 AMainmenu::getCivPoints()
{
	if (Civilization)
	{
		return Civilization->LeftOverPoints;
	}

	return 0;
}

TArray<FString> AMainmenu::getCivTraits()
{
	if(Civilization)
	{
		return Civilization->TakenTraits;
	}

	return TArray<FString>();
}
