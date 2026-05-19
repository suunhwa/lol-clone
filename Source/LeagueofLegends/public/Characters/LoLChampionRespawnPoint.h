// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Type/RiftTypes.h"
#include "LoLChampionRespawnPoint.generated.h"

// 팀 별 부활 우물 위치 마커
// 맵에 블루/레드 각 1개씩 배치
// SlotIndex 기반 offset으로 팀원 위치 분산
UCLASS()
class LEAGUEOFLEGENDS_API ALoLChampionRespawnPoint : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALoLChampionRespawnPoint();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Respawn")
	ETeam Team = ETeam::None;
	
	// SlotIndex 오프셋 배열 (최대 5인 기준)
	static const TArray<FVector> SlotOffsets;
	
	
};
