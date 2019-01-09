// Fill out your copyright notice in the Description page of Project Settings.

#include "BuildableTile.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

ABuildableTile::ABuildableTile()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
	TileMesh->SetCollisionProfileName("NoCollision");
	TileMesh->SetupAttachment(Root);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetRelativeLocation(FVector(0, 0, 50));
	CollisionBox->SetBoxExtent(FVector(50, 50, 50));
	CollisionBox->SetupAttachment(RootComponent);
}

void ABuildableTile::BeginPlay()
{
	Super::BeginPlay();
	
	DefaultMaterial = TileMesh->GetMaterial(0);
}

void ABuildableTile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABuildableTile::SetIsGhost()
{
	bIsGhost = true;

	TileMesh->SetMaterial(0, GhostMaterial);

	DynMaterial = TileMesh->CreateDynamicMaterialInstance(0);
}

void ABuildableTile::Place(TArray<FVector2D> nPlacedOnTiles)
{
	PlacedOnTiles = nPlacedOnTiles;

	TileMesh->SetMaterial(0, DefaultMaterial);
	bIsGhost = false;
}

void ABuildableTile::SetIsBlocked(bool bBlocked)
{
	if(DynMaterial)
	{
		float v = bBlocked ? 0.0f : 1.0f;
		DynMaterial->SetScalarParameterValue("bBlocked", v);
	}
}
