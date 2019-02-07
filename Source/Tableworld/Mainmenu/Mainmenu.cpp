// Copyright by Robert Vater (Gunschlinger)

#include "Mainmenu.h"
#include "Misc/Math/FastNoise.h"
#include "World/TableWorldTable.h"

void AMainmenu::BeginPlay()
{
	Super::BeginPlay();

	int32 TextureSize = WorldX * 16;

	if (!MapPreview)
	{
		//Create the MapPreview texture
		MapPreview = UTexture2D::CreateTransient(TextureSize, TextureSize);
		MapPreview->AddToRoot();
		MapPreview->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
		MapPreview->SRGB = false;
		MapPreview->Filter = TextureFilter::TF_Nearest;
		MapPreview->UpdateResource();
	}
}

void AMainmenu::LeaveMainmenu()
{
	if(MapPreview)
	{
		MapPreview->RemoveFromRoot();
		MapPreview = nullptr;
	}
}

void AMainmenu::SetSeed(int32 nUsedSeed)
{
	UsedSeed = nUsedSeed;
}

void AMainmenu::SetWorldSize(uint8 nWorldX, uint8 nWorldY)
{
	WorldX = nWorldX;
	WorldY = nWorldY;
}

UTexture2D* AMainmenu::GenerateMapPreview()
{
	int32 TextureSize = WorldX * 16;

	if (MapPreview) 
	{
		UFastNoise* Noise = NewObject<UFastNoise>(this);
		Noise->SetFrequency(0.05f);
		Noise->SetSeed(UsedSeed);

		uint8* Pixels = new uint8[TextureSize * TextureSize * 4];
		for (int32 x = 0; x < TextureSize; x++)
		{
			for (int32 y = 0; y < TextureSize; y++)
			{
				float NoiseLevel = Noise->GetNoise2D(x, y);
				FColor PixelColor = FColor::White;

				if (NoiseLevel < ATableWorldTable::SandLevel)
				{
					PixelColor = FColor::Yellow;
				}

				if (NoiseLevel < ATableWorldTable::WaterLevel)
				{
					PixelColor = FColor::Blue;
				}

				if (NoiseLevel > ATableWorldTable::RockLevel)
				{
					PixelColor = FColor(100, 100, 100);
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
