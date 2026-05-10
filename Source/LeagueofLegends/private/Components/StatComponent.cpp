// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/StatComponent.h"

#include "LeagueofLegends.h"
#include "Net/UnrealNetwork.h"

UStatComponent::UStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UStatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UStatComponent, CurrentHP);
	DOREPLIFETIME(UStatComponent, CurrentMana);
	DOREPLIFETIME(UStatComponent, Level);
	DOREPLIFETIME(UStatComponent, CachedMaxHP);
	DOREPLIFETIME(UStatComponent, CachedMaxMana);
}

void UStatComponent::InitStats(const FChampionBaseRow& ChampionBase, const FChampionStatRow& Stats,
                               const FChampionGrowthRow& Growth)
{
	Level = 1; // 항상 1레벨로 초기화 (Blueprint 기본값 오염 방지)

	BaseMoveSpeed = ChampionBase.MoveSpeed;
	BaseRange = ChampionBase.AttackRange;

	BaseHP = Stats.HP;
	BaseMana = Stats.MP;
	BaseAD = Stats.AD;
	BaseAP = Stats.AP;
	BaseArmor = Stats.Armor;
	BaseMR = Stats.MR;
	BaseAS = Stats.AS_Base;
	BaseASRatio = Stats.AS_Ratio;

	BaseHPRegen = Stats.Regen_HP;
	BaseCritMult = Stats.Crit_Mult > 0.f ? Stats.Crit_Mult : 1.75f;

	HP_G = Growth.HP_G;
	Mana_G = Growth.MP_G;
	AD_G = Growth.AD_G;
	Armor_G = Growth.Armor_G;
	MR_G = Growth.MR_G;
	AS_G = Growth.AS_G_Pct;
	AP_G = Growth.AP_G;
	MS_G = Growth.MS_G;
	HPRegen_G = Growth.Regen_HP_G;

	CachedMaxHP = BaseHP + HP_G * (Level - 1) + BonusHP;
	CachedMaxMana = BaseMana + Mana_G * (Level - 1);
	CurrentHP = CachedMaxHP;
	CurrentMana = CachedMaxMana;

	OnHPChanged.Broadcast(CurrentHP, CachedMaxHP);
	OnManaChanged.Broadcast(CurrentMana, GetMaxMana());
	OnLevelChanged.Broadcast(Level);
}


float UStatComponent::GetAD() const
{
	return BaseAD + AD_G * (Level - 1) + BonusAD;
}

float UStatComponent::GetAP() const
{
	return BaseAP + BonusAP;
}

float UStatComponent::GetArmor() const
{
	return BaseArmor + Armor_G * (Level - 1) + BonusArmor;
}

float UStatComponent::GetMagicResist() const
{
	return BaseMR + MR_G * (Level - 1);
}

float UStatComponent::GetMoveSpeed() const
{
	return BaseMoveSpeed + MS_G * (Level - 1);
}

float UStatComponent::GetHPRegen() const
{
	return BaseHPRegen + HPRegen_G * (Level - 1);
}

float UStatComponent::GetAttackSpeed() const
{
	// LoL 공격속도 공식: BaseAS + BaseAS * (ASRatio * AS_G * (Level-1) / 100)
	return BaseAS + BaseAS * (BaseASRatio * AS_G * (Level - 1) / 100.f);
}

float UStatComponent::GetAttackRange() const
{
	return BaseRange;
}

void UStatComponent::ApplyHealthChange(float Delta)
{
	CurrentHP = FMath::Clamp(CurrentHP + Delta, 0.f, GetMaxHP());
	OnHPChanged.Broadcast(CurrentHP, GetMaxHP());
}

void UStatComponent::ApplyManaCost(float Cost)
{
	CurrentMana = FMath::Clamp(CurrentMana - Cost, 0.f, GetMaxMana());
	OnManaChanged.Broadcast(CurrentMana, GetMaxMana());
}

void UStatComponent::SetLevel(int32 NewLevel)
{
	PRINTLOG_SH(TEXT("[SetLevel] %d → %d (Owner: %s)"), Level, NewLevel, *GetNameSafe(GetOwner()));
	const float HPRatio = CachedMaxHP > 0.f ? CurrentHP / CachedMaxHP : 1.f;
	const float ManaRatio = CachedMaxMana > 0.f ? CurrentMana / CachedMaxMana : 1.f;

	Level = FMath::Clamp(NewLevel, 1, 18);
	CachedMaxHP = BaseHP + HP_G * (Level - 1) + BonusHP;
	CachedMaxMana = BaseMana + Mana_G * (Level - 1);

	CurrentHP = CachedMaxHP * HPRatio;
	CurrentMana = CachedMaxMana * ManaRatio;

	OnHPChanged.Broadcast(CurrentHP, CachedMaxHP);
	OnManaChanged.Broadcast(CurrentMana, CachedMaxMana);
	OnLevelChanged.Broadcast(Level);
}

void UStatComponent::OnRep_CurrentHP()
{
	OnHPChanged.Broadcast(CurrentHP, GetMaxHP());
}

void UStatComponent::OnRep_CurrentMana()
{
	OnManaChanged.Broadcast(CurrentMana, GetMaxMana());
}

void UStatComponent::OnRep_CachedMaxMana()
{
	OnManaChanged.Broadcast(CurrentMana, CachedMaxMana);
}

void UStatComponent::OnRep_Level()
{
	OnLevelChanged.Broadcast(Level);
}
