// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CooldownComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LEAGUEOFLEGENDS_API UCooldownComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCooldownComponent();

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

public:
	void StartCooldown(FName Tag, float Duration);
	bool IsOnCooldown(FName Tag) const;
	float GetRemaining(FName Tag) const;
	float GetDuration(FName Tag) const;
	void ReduceAllCooldowns(float Amount); // Q 패시브 등 쿨다운 감소

	// AH → CDR 변환: CDR% = AH / (100 + AH)
	void SetAbilityHaste(float InAH) { AbilityHaste = FMath::Max(0.f, InAH); }

private:
	TMap<FName, float> Cooldowns;
	TMap<FName, float> Durations; // 총 쿨타임 (퍼센트 계산용)

	float AbilityHaste = 0.f;

	float ApplyCDR(float BaseDuration) const { return BaseDuration * (100.f / (100.f + AbilityHaste)); }
};
