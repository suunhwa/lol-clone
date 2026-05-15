// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/RiftTypes.h"
#include "StatusEffectComponent.generated.h"

USTRUCT()
struct FActiveStatusEffect
{
	GENERATED_BODY()

	EStatusEffect Type = EStatusEffect::Stun;
	float Duration = 0.f;
	float Remaining = 0.f;
	float Magnitude = 1.f; // Slow에서 감속 비율 (0~1)
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LEAGUEOFLEGENDS_API UStatusEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStatusEffectComponent();

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

public:
	// 서버에서만 호출. Instigator: CC를 건 챔피언 액터 (어시스트 기록용)
	void ApplyEffect(EStatusEffect Type, float Duration, float Magnitude = 1.f, AActor* Instigator = nullptr);
	void RemoveEffect(EStatusEffect Type);
	void RemoveAll();

	bool HasEffect(EStatusEffect Type) const;
	float GetSlowMagnitude() const; // 가장 강한 Slow 반환 (0~1, 클수록 느림)

	// StateComponent가 전환 가능 여부를 물어볼 때 사용
	bool CanMove() const;
	bool CanAttack() const;
	bool CanCastSkill() const;

private:
	TArray<FActiveStatusEffect> ActiveEffects;
	
	// CC 적용 시 가해자를 Owner의 CombatComp에 어시스트로 기록
	void RegisterCCAssist(AActor* Instigator);
};
