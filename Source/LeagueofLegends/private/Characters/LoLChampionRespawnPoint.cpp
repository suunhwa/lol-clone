// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/LoLChampionRespawnPoint.h"

#include "Components/BillboardComponent.h"

const TArray<FVector> ALoLChampionRespawnPoint::SlotOffsets = {
	FVector(0.f, 0.f, 0.f),
	FVector(120.f, 0.f, 0.f),
	FVector(-120.f, 0.f, 0.f),
	FVector(0.f, 120.f, 0.f),
	FVector(0.f, -120.f, 0.f),
};

// Sets default values
ALoLChampionRespawnPoint::ALoLChampionRespawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;
	
#if WITH_EDITOR
	// 에디터에서 위치 확인용 빌보드
	if (UBillboardComponent* Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard")))
	{
		RootComponent = Billboard;
	}
#endif
	
}


