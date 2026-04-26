// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TargetingComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LEAGUEOFLEGENDS_API UTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTargetingComponent();

public:
	// 해당 유닛을 타겟으로 잡을 수 있는지
	bool IsValidTarget(AActor* Attacker) const;

	// 해당 유닛의 공격 우선순위
	// 높을수록 먼저 공격
	int32 GetPriority(AActor* Attacker) const;
};
