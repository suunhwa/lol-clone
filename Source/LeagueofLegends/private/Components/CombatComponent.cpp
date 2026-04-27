// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/CombatComponent.h"
#include "Components/StatComponent.h"
#include "Components/StateComponent.h"
#include "Components/TagComponent.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UCombatComponent::DealDamage(AActor* Target, FDamageContext Ctx)
{
	if (!GetOwner()->HasAuthority() || !Target) return;

	UStatComponent* TargetStat = Target->FindComponentByClass<UStatComponent>();
	if (!TargetStat || TargetStat->IsDead()) return;

	float FinalDamage = CalculateFinalDamage(Ctx, Target);

	TargetStat->ApplyHealthChange(-FinalDamage);

	UStateComponent* TargetState = Target->FindComponentByClass<UStateComponent>();
	if (TargetState)
		TargetState->TryChangeState(ECharacterState::Hit);

	// 흡혈
	if (Ctx.LifeStealRatio > 0.f)
	{
		UStatComponent* OwnerStat = GetOwner()->FindComponentByClass<UStatComponent>();
		if (OwnerStat)
			OwnerStat->ApplyHealthChange(FinalDamage * Ctx.LifeStealRatio);
	}

	OnDamageDealt.Broadcast(Target, FinalDamage);

	if (TargetStat->IsDead())
	{
		if (TargetState)
			TargetState->TryChangeState(ECharacterState::Dead);

		UTagComponent* TargetTag = Target->FindComponentByClass<UTagComponent>();
		if (TargetTag)
		{
			TargetTag->AddTag(UnitTags::Dead);
			TargetTag->AddTag(UnitTags::Untargetable);
		}

		UCombatComponent* TargetCombat = Target->FindComponentByClass<UCombatComponent>();
		if (TargetCombat)
			TargetCombat->OnDeath.Broadcast(Ctx.DamageInstigator);
	}
}

void UCombatComponent::PerformBasicAttack(AActor* Target)
{
	UStatComponent* OwnerStat = GetOwner()->FindComponentByClass<UStatComponent>();
	if (!OwnerStat) return;

	FDamageContext Ctx;
	Ctx.RawDamage = OwnerStat->GetAD();
	Ctx.DamageType = EDamageType::Physical;
	Ctx.DamageInstigator = GetOwner();
	Ctx.SourceTag = TEXT("BasicAttack");

	DealDamage(Target, Ctx);
}

float UCombatComponent::CalculateFinalDamage(const FDamageContext& Ctx, AActor* Target) const
{
	if (Ctx.DamageType == EDamageType::TrueDamage) return Ctx.RawDamage;

	UStatComponent* TargetStat = Target->FindComponentByClass<UStatComponent>();
	if (!TargetStat) return Ctx.RawDamage;

	float Resistance = (Ctx.DamageType == EDamageType::Physical)
		                   ? TargetStat->GetArmor() * Ctx.ArmorPenRatio - Ctx.ArmorPenFlat
		                   : TargetStat->GetMagicResist();

	Resistance = FMath::Max(Resistance, -100.f);

	float Reduced = (Resistance >= 0.f)
		                ? Ctx.RawDamage * 100.f / (100.f + Resistance)
		                : Ctx.RawDamage * (2.f - 100.f / (100.f - Resistance));

	if (Ctx.bIsCritical)
	{
		UStatComponent* OwnerStat = GetOwner()->FindComponentByClass<UStatComponent>();
		if (OwnerStat) Reduced *= OwnerStat->GetCritMultiplier();
	}

	return Reduced;
}
