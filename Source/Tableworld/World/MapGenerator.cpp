// Copyright by Robert Vater (Gunschlinger)

#include "MapGenerator.h"


uint8 MapGenerator::TileSize= 100;
uint8 MapGenerator::ChunkSize = 16;
uint8 MapGenerator::RiverSize = 2;

//Base tile Noise level
float MapGenerator::WaterLevel = 0.25f;
float MapGenerator::SandLevel = 0.15f;
float MapGenerator::RockLevel = 0.3f;

//Rescource Noise Level
float MapGenerator::TreeValue = 0.1f;
float MapGenerator::CoppperValue = 0.5f;
float MapGenerator::TinValue = 0.5f;
float MapGenerator::BerrieValue = 0.55f;


MapGenerator::MapGenerator()
{

}

TArray<FGeneratedMapTile> MapGenerator::GenerateMap(int32 Seed, int32 WorldSize, bool bHasRiver, uint8 RiverCount)
{
	TArray<FGeneratedMapTile> Tiles;

	UFastNoise* Noise = NewObject<UFastNoise>();
	Noise->SetSeed(Seed);

	Noise->SetNoiseType(ENoiseType::SimplexFractal);
	Noise->SetFrequency(0.015f);

	FMath::RandInit(Seed);

	int32 MaxTiles = ChunkSize * WorldSize;
	
	//Generate the default tiles
	for(int32 y = 0; y < MaxTiles; y++)
	{
		for (int32 x = 0; x < MaxTiles; x++)
		{
			Tiles.Add(CreateTile(x, y));
		}
	}

	//Generate BaseTiles
	for(int32 y = 0; y < MaxTiles; y++)
	{
		for(int32 x = 0; x < MaxTiles; x++)
		{
			float NoiseValue = Noise->GetNoise2D(x, y);
			SetTile(Tiles, WorldSize, x, y);

			//Place Rock
			if(NoiseValue >= RockLevel)
			{
				SetTile(Tiles, WorldSize, x, y, ETileType::Rock, ETileRescources::None, ETileType::Max, NoiseValue);

				int32 dx = FMath::RandRange(-1, 1);
				int32 dy = FMath::RandRange(-1, 1);
				SetTile(Tiles, WorldSize, x + dx, y + dy, ETileType::Rock, ETileRescources::None, ETileType::Grass, NoiseValue);
			}
		}
	}

	Noise->SetSeed(FMath::RandRange(0, FMath::Rand()));
	Noise->SetFrequency(0.015f);

	for (int32 y = 0; y < MaxTiles; y++)
	{
		for (int32 x = 0; x < MaxTiles; x++)
		{
			float NoiseValue = Noise->GetNoise2D(x, y);
			
			//Place Sand
			if (NoiseValue >= SandLevel)
			{
				SetTile(Tiles, WorldSize, x, y, ETileType::Sand, ETileRescources::None, ETileType::Max, NoiseValue);

				int32 dx = FMath::RandRange(-1, 1);
				int32 dy = FMath::RandRange(-1, 1);
				SetTile(Tiles, WorldSize, x, y, ETileType::Sand, ETileRescources::None, ETileType::Grass, NoiseValue);
			}

			//Place Water
			if (NoiseValue >= WaterLevel)
			{
				SetTile(Tiles, WorldSize, x, y, ETileType::Water, ETileRescources::None, ETileType::Max, NoiseValue);
			}
		}
	}

	if(bHasRiver)
	{
		//Generate the rivers
		int32 NumRivers = RiverCount;
		bool bYRiver = FMath::RandBool();

		TArray<FGeneratedMapTile> RiverTiles;

		for (int32 i = 0; i < NumRivers; i++)
		{
			int32 RiverStartX = 0;
			int32 RiverStartY = 0;

			int32 RiverEndX = 0;
			int32 RiverEndY = 0;

			int32 RiverSteps = 0;
			int32 MaxRiverSteps = FMath::RandRange(1000, 5000);

			if(bYRiver)
			{
				//River starting at Y 0
				RiverStartY = 0;
				RiverStartX = FMath::RandRange(0, MaxTiles);

				RiverEndY = MaxTiles;
				RiverEndX = FMath::RandRange(0, MaxTiles);
			}else
			{
				//River starting at X 0
				RiverStartX = 0;
				RiverStartY = FMath::RandRange(0, MaxTiles);

				RiverEndX = MaxTiles;
				RiverEndY = FMath::RandRange(0, MaxTiles);
			}

			int32 CurrentRiverSize = FMath::RandRange(2,3);
			int32 RiverRSizeNext = 0;

			int32 CurrentX = RiverStartX;
			int32 CurrentY = RiverStartY;

			while(RiverSteps < MaxRiverSteps)
			{
				if (CurrentX < RiverEndX)CurrentX++;
				if (CurrentX > RiverEndX)CurrentX--;

				if (CurrentY < RiverEndY)CurrentY++;
				if (CurrentY > RiverEndY)CurrentY--;

				CurrentX += FMath::RandRange(-1, 1);
				CurrentY += FMath::RandRange(-1, 1);

				FGeneratedMapTile NewTile;
				NewTile.X = CurrentX;
				NewTile.Y = CurrentY;
				RiverTiles.Add(NewTile);

				//Range Tiles
				for(int32 x = 0; x < CurrentRiverSize; x++)
				{
					for(int32 y = 0; y < CurrentRiverSize; y++)
					{
						FGeneratedMapTile NewTile;
						NewTile.X = CurrentX + x;
						NewTile.Y = CurrentY + y;
						RiverTiles.Add(NewTile);
					}
				}

				if(RiverRSizeNext >= FMath::RandRange(5,10))
				{
					RiverRSizeNext = 0;

					CurrentRiverSize = FMath::RandRange(1, 3);
				}

				if(CurrentX == RiverEndX && CurrentY == RiverEndY)
				{
					break;
				}

				RiverSteps++;
			}
		}

		for(int32 i = 0; i < RiverTiles.Num(); i++)
		{
			FGeneratedMapTile Tile = RiverTiles[i];
			SetTile(Tiles, WorldSize, Tile.X, Tile.Y, ETileType::Water);
		}

		RiverTiles.Empty();
	}

	Noise->SetFrequency(0.025f);

	for (int32 y = 0; y < MaxTiles; y++)
	{
		for (int32 x = 0; x < MaxTiles; x++)
		{
			float NoiseValue = Noise->GetNoise2D(x, y);
			
			//Place trees
			if (NoiseValue >= TreeValue)
			{
				SetTile(Tiles, WorldSize, x, y, ETileType::Grass, ETileRescources::Tree, ETileType::Grass, NoiseValue);
			}
		}
	}

	Noise->SetSeed(FMath::Rand());

	for (int32 y = 0; y < MaxTiles; y++)
	{
		for (int32 x = 0; x < MaxTiles; x++)
		{
			float NoiseValue = Noise->GetNoise2D(x, y);

			if (NoiseValue >= TinValue)
			{
				SetRescourceInClump(Tiles, WorldSize, x, y, FMath::RandRange(1, 2), ETileRescources::TinOre, ETileType::Grass);
			}

			if (NoiseValue >= CoppperValue)
			{
				SetRescourceInClump(Tiles, WorldSize, x, y, FMath::RandRange(1, 2), ETileRescources::CopperOre, ETileType::Grass);
			}
		}
	}

	Noise->SetFrequency(0.7f);

	for (int32 y = 0; y < MaxTiles; y++)
	{
		for (int32 x = 0; x < MaxTiles; x++)
		{
			float NoiseValue = Noise->GetNoise2D(x, y);

			//Place Berries
			if (NoiseValue >= BerrieValue)
			{
				SetRescourceInClump(Tiles, WorldSize, x, y, FMath::RandRange(2, 4), ETileRescources::Berries, ETileType::Grass);
			}
		}
	}

	return Tiles;
}

int32 MapGenerator::getTileIndex(int32 X, int32 Y, int32 WorldSize)
{
	return Y * (ChunkSize * WorldSize) + X;
}

FGeneratedMapTile MapGenerator::CreateTile(int32 X, int32 Y, ETileType Type, ETileRescources Res, uint8 Amount)
{
	FGeneratedMapTile Tile;
	Tile.X = X;
	Tile.Y = Y;
	Tile.TileType = Type;
	Tile.Resscource = Res;

	return Tile;
}

FGeneratedMapTile MapGenerator::getTile(TArray<FGeneratedMapTile> Array, uint8 Size, int32 X, int32 Y)
{
	int32 Index = getTileIndex(X, Y, Size);

	FGeneratedMapTile Tile;
	if(Array.IsValidIndex(Index))
	{
		return Array[Index];
	}

	return Tile;
}

void MapGenerator::SetTile(TArray<FGeneratedMapTile>& Array, int32 Size, int32 X, int32 Y, ETileType Type, ETileRescources Res, ETileType IfType, float NoiseValue)
{
	int32 TileIndex = getTileIndex(X, Y, Size);
	if(Array.IsValidIndex(TileIndex))
	{
		FGeneratedMapTile CurrentTile = Array[TileIndex];
		if(IfType != ETileType::Max)
		{
			if(CurrentTile.Resscource != ETileRescources::None && CurrentTile.Resscource != ETileRescources::Max)
			{
				return;
			}
			
			if(CurrentTile.TileType != IfType)
			{
				return;
			}
		}

		if(Type != ETileType::Max)
		{
			CurrentTile.TileType = Type;
		}

		if(Res != ETileRescources::None)
		{
			CurrentTile.Resscource = Res;
		}

		if(CurrentTile.NoiseValue != NoiseValue)
		{
			CurrentTile.NoiseValue = NoiseValue;
		}

		Array[TileIndex] = CurrentTile;
	}
}

void MapGenerator::SetRescourceInClump(TArray<FGeneratedMapTile>& Array, int32 Size, int32 X, int32 Y, int32 Radius, ETileRescources Res, ETileType IfTile /*= ETileType::Max*/)
{
	for (int32 x = -Radius; x <= Radius; x++)
	{
		for (int32 y = -Radius; y <= Radius; y++)
		{
			if ((x * x) + (y * y) < (Radius * Radius))
			{
				int32 XX = (X + x);
				int32 YY = (Y + y);

				SetTile(Array, Size, XX, YY, ETileType::Max, Res, IfTile);
			}
		}
	}
}

bool MapGenerator::getTileArea(TArray<FGeneratedMapTile>& Array, int32 Size, TMap<ETileType, int32> RequiredTiles, int32 SizeX, int32 SizeY, int32& OutX, int32& OutY)
{
	

	return false;
}

FColor MapGenerator::getTileColor(ETileType Type, ETileRescources Rescource /*= ETileRescources::None*/)
{
	FColor Color = FColor::White;
	
	switch(Type)
	{
		case ETileType::Grass: Color = FColor::FromHex("#263617"); break;
		case ETileType::Dirt: Color = FColor::Orange; break;
		case ETileType::Rock: Color = FColor(100, 100, 100); break;
		case ETileType::Sand: Color = FColor::FromHex("#c2b280"); break;
		case ETileType::Water: Color = FColor::FromHex("#0077be"); break;
		case ETileType::DirtRoad: Color = FColor::Red; break;
	}

	if (Rescource != ETileRescources::None && Rescource != ETileRescources::Max)
	{
		switch (Rescource)
		{
			case ETileRescources::Tree: Color = FColor::FromHex("#006400"); break;
			case ETileRescources::Berries: Color = FColor::Purple; break;
			case ETileRescources::TinOre: Color = FColor::FromHex("#413e44"); break;
			case ETileRescources::CopperOre: Color = FColor::Orange; break;
		}
	}

	return Color;
}
