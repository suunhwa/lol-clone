// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/RiftTypes.h"
#include "CombatComponent.generated.h"

USTRUCT(BlueprintType)
struct FDamageContext
{
	GENERATED_BODY()

	float RawDamage = 0.f;
	EDamageType DamageType = EDamageType::Physical;
	bool bIsCritical = false;
	float ArmorPenFlat = 0.f;
	float ArmorPenRatio = 1.f; // 1 = 관통 없음
	float LifeStealRatio = 0.f;
	AActor* DamageInstigator = nullptr;
	FName SourceTag; // "BasicAttack", "Ezreal.Q" ...
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDamageDealt, AActor* /*Target*/, float /*FinalDamage*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDamageReceived, AActor* /*DamageInstigator*/, float /*FinalDamage*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor* /*DamageInstigator*/);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LEAGUEOFLEGENDS_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

public:
	// Owner의 CombatComp가 Target에게 데미지 적용 (서버 전용)
	void DealDamage(AActor* Target, FDamageContext Ctx);

	// 평타 (애니 노티파이에서 호출)
	void PerformBasicAttack(AActor* Target);

	FOnDamageDealt OnDamageDealt;
	FOnDamageReceived OnDamageReceived;
	FOnDeath OnDeath;

private:
	float CalculateFinalDamage(const FDamageContext& Ctx, AActor* Target) const;
};
