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

	float RYaw = FMath::RandRange(0, 360);
	TileMesh->SetRelativeRotation(FRotator(0, RYaw, 0));
}

void ABuildableTile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABuildableTile::SetIsGhost()
{
	bIsGhost = true;

	TileMesh->SetMaterial(0, GhostMaterial);
}

void ABuildableTile::Place(int32 X, int32 Y)
{
	TileX = X;
	TileY = Y;

	TileMesh->SetMaterial(0, DefaultMaterial);
	bIsGhost = false;
}

