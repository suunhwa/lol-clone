// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Struct/ChampionStatStruct.h"
#include "Type/StatModifierTypes.h"
#include "StatComponent.generated.h"

class UStatModifierComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHPChanged, float /*Current*/, float /*Max*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnManaChanged, float /*Current*/, float /*Max*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnLevelChanged, int32 /*NewLevel*/);

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

	// LoLChampion이 소유한 StatModifierComp를 StatComponent에 주입
	void SetStatModifierComp(UStatModifierComponent* InComp) { StatModifierComp = InComp; }

	// --- getters 
	float GetCurrentHP() const { return CurrentHP; }
	float GetCurrentMana() const { return CurrentMana; }
	int32 GetLevel() const { return Level; }

	float GetMaxHP() const { return CachedMaxHP; }
	float GetMaxMana() const { return CachedMaxMana; }
	float GetAD() const;
	float GetAP() const;
	float GetArmor() const;
	float GetMagicResist() const;
	float GetMoveSpeed() const;
	float GetAttackSpeed() const;
	float GetAttackRange() const;
	float GetHPRegen() const;
	float GetAbilityHaste() const;
	float GetCritChance() const;
	float GetCritMultiplier() const { return BaseCritMult; }

	bool IsDead() const { return CurrentHP <= 0.f; }

	// --- modifiers (서버에서만 호출)
	void ApplyHealthChange(float Delta); // + heal, - damage
	void ApplyManaCost(float Cost); // 항상 양수로 전달
	void SetLevel(int32 NewLevel);

	// HP/Mana 관련 모디파이어 추가/제거 후 외부에서 호출
	void RecalcMaxHP();
	void RecalcMaxMana();

	// --- delegates 
	FOnHPChanged OnHPChanged;
	FOnManaChanged OnManaChanged;
	FOnLevelChanged OnLevelChanged;

	// 자식 접근을 위해 공통변수로 뺌
protected:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHP)
	float CurrentHP = 0.f;

	UPROPERTY(Replicated)
	float CachedMaxHP = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_CachedMaxMana)
	float CachedMaxMana = 0.f;
	// ------------------------
private:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentMana)
	float CurrentMana = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_Level)
	int32 Level = 1;

	// 자식 접근을 위해 공통변수로 뺌
protected:
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
	// --------------------------

private:
	float BaseHPRegen = 0.f;
	float BaseCritMult = 1.75f; // LoL 기본 크리 배율

	// 성장치 (레벨업마다 적용)
	// 자식 접근을 위해 공통변수로 뺌
protected:
	float HP_G = 0.f;
	float AD_G = 0.f;
	// --------------------------
private:
	float Mana_G = 0.f;
	float Armor_G = 0.f;
	float MR_G = 0.f;
	float AS_G = 0.f;
	float AP_G = 0.f;
	float MS_G = 0.f;
	float HPRegen_G = 0.f;

	// LoLChampion이 소유 — StatComponent는 참조만 보관 (비소유)
	UPROPERTY()
	TObjectPtr<UStatModifierComponent> StatModifierComp = nullptr;

	UFUNCTION()
	void OnRep_CurrentHP();

	UFUNCTION()
	void OnRep_CurrentMana();

	UFUNCTION()
	void OnRep_CachedMaxMana();

	UFUNCTION()
	void OnRep_Level();
};
