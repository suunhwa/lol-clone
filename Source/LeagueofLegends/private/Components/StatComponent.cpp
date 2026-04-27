// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/StatComponent.h"
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
}

void UStatComponent::InitStats(const FChampionBaseRow& ChampionBase, const FChampionStatRow& Stats,
                               const FChampionGrowthRow& Growth)
{
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

	CurrentHP = GetMaxHP();
	CurrentMana = GetMaxMana();
}

float UStatComponent::GetMaxHP() const
{
	return BaseHP + HP_G * (Level - 1) + BonusHP;
}

float UStatComponent::GetMaxMana() const
{
	return BaseMana + Mana_G * (Level - 1);
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
	Level = FMath::Clamp(NewLevel, 1, 18);
}

void UStatComponent::OnRep_CurrentHP()
{
	OnHPChanged.Broadcast(CurrentHP, GetMaxHP());
}

void UStatComponent::OnRep_CurrentMana()
{
	OnManaChanged.Broadcast(CurrentMana, GetMaxMana());
}
