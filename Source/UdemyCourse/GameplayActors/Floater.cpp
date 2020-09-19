// Fill out your copyright notice in the Description page of Project Settings.


#include "Floater.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AFloater::AFloater()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CustomStaticMesh"));

}

// Called when the game starts or when spawned
void AFloater::BeginPlay()
{
	Super::BeginPlay();

	float InitialX = FMath::FRand();
	float InitialY = FMath::FRand();
	float InitialZ = FMath::FRand();

	InitialLocation.X = 20 * InitialX;
	InitialLocation.Y = 20 * InitialY;
	InitialLocation.Z = 20 * InitialZ;

	SetActorLocation(InitialLocation);
}

// Called every frame
void AFloater::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

