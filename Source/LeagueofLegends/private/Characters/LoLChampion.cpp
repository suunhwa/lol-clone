// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/LoLChampion.h"

#include "Characters/Data/ChampionData.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"

ALoLChampion::ALoLChampion()
{
	PrimaryActorTick.bCanEverTick = true;
	
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(
		TEXT("/Game/Champions/Ezreal/SkeletalMeshes/Ezreal.Ezreal")
	);

	if (MeshAsset.Succeeded())
	{
		GetMesh()->SetSkeletalMeshAsset(MeshAsset.Object);
		GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
		GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
		GetMesh()->SetRelativeScale3D(FVector(0.008f));
	}
}

void ALoLChampion::BeginPlay()
{
	Super::BeginPlay();
	
	if (!ChampionData) return;
}

void ALoLChampion::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ALoLChampion, ChampionData);
}

void ALoLChampion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALoLChampion::OnRep_ChampionData()
{
	InitVisuals();
}

void ALoLChampion::InitVisuals()
{
	
}

void ALoLChampion::InitStats()
{
}


