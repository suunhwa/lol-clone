#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Struct/ObjectStruct.h"
#include "ObjectStatComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnObjectHPChanged, float /*Current*/, float /*Max*/);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LEAGUEOFLEGENDS_API UObjectStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UObjectStatComponent();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	void InitObjectStats(const FObjectBaseRow& Base, const FObjectRewardRow& Reward, const FObjectMechanicsRow& Mech);

	// Getters
	float GetCurrentHP() const { return CurrentHP; }
	float GetMaxHP() const { return BaseData.Base_HP; }
	float GetAttackDamage() const; // 가열 적용 대미지
	float GetAttackRange() const { return BaseData.Atk_Range; }
	float GetAttackSpeed() const { return BaseData.Atk_Speed; }
	float GetArmor() const { return CurrentArmor; }
	bool IsDead() const { return CurrentHP <= 0.f; }

	// Heating System
	void AddHeatingStack();
	void ResetHeatingStack();
	float GetCurrentHeatingMultiplier() const;

	void ApplyDamage(float Amount);

	FOnObjectHPChanged OnHPChanged;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHP)
	float CurrentHP = 0.f;

	UPROPERTY(Replicated)
	float CurrentArmor = 0.f;

	UPROPERTY(Replicated)
	int32 CurrentHeatStack = 0;

	FObjectBaseRow BaseData;
	FObjectRewardRow RewardData;
	FObjectMechanicsRow MechData;

	UFUNCTION()
	void OnRep_CurrentHP();
};