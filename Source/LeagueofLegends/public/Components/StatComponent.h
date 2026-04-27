// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Struct/ChampionStatStruct.h"
#include "StatComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHPChanged, float /*Current*/, float /*Max*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnManaChanged, float /*Current*/, float /*Max*/);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LEAGUEOFLEGENDS_API UStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStatComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	void InitStats(const FChampionBaseRow& ChampionBase, const FChampionStatRow& Stats,
	               const FChampionGrowthRow& Growth);

	// --- getters 
	float GetCurrentHP() const { return CurrentHP; }
	float GetCurrentMana() const { return CurrentMana; }
	int32 GetLevel() const { return Level; }

	float GetMaxHP() const;
	float GetMaxMana() const;
	float GetAD() const;
	float GetAP() const;
	float GetArmor() const;
	float GetMagicResist() const;
	float GetMoveSpeed() const;
	float GetAttackSpeed() const;
	float GetAttackRange() const;
	float GetHPRegen() const;
	float GetCritMultiplier() const { return BaseCritMult; }

	bool IsDead() const { return CurrentHP <= 0.f; }

	// --- modifiers (서버에서만 호출)
	void ApplyHealthChange(float Delta); // + heal, - damage
	void ApplyManaCost(float Cost); // 항상 양수로 전달
	void SetLevel(int32 NewLevel);

	// 보너스 스탯 (아이템/버프)
	void AddBonusAD(float Value) { BonusAD += Value; }
	void AddBonusAP(float Value) { BonusAP += Value; }
	void AddBonusHP(float Value) { BonusHP += Value; }
	void AddBonusArmor(float Value) { BonusArmor += Value; }

	// --- delegates 
	FOnHPChanged OnHPChanged;
	FOnManaChanged OnManaChanged;

private:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHP)
	float CurrentHP = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentMana)
	float CurrentMana = 0.f;

	UPROPERTY(Replicated)
	int32 Level = 1;

	// 기본 스탯 (DataAsset 연결 전 InitStats로 세팅)
	float BaseHP = 0.f;
	float BaseMana = 0.f;
	float BaseAD = 0.f;
	float BaseAP = 0.f;
	float BaseArmor = 0.f;
	float BaseMR = 0.f;
	float BaseMoveSpeed = 0.f;
	float BaseAS = 0.f;
	float BaseASRatio = 0.f;
	float BaseRange = 0.f;

	float BaseHPRegen = 0.f;
	float BaseCritMult = 1.75f; // LoL 기본 크리 배율

	// 성장치 (레벨업마다 적용)
	float HP_G = 0.f;
	float Mana_G = 0.f;
	float AD_G = 0.f;
	float Armor_G = 0.f;
	float MR_G = 0.f;
	float AS_G = 0.f;
	float AP_G = 0.f;
	float MS_G = 0.f;
	float HPRegen_G = 0.f;

	// 보너스 스탯 (아이템/버프)
	float BonusHP = 0.f;
	float BonusAD = 0.f;
	float BonusAP = 0.f;
	float BonusArmor = 0.f;

	UFUNCTION()
	void OnRep_CurrentHP();

	UFUNCTION()
	void OnRep_CurrentMana();
};
