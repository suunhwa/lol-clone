// Fill out your copyright notice in the Description page of Project Settings.

#include "Champions/Ezreal/EzrealSkillExecutor.h"

#include "Engine/OverlapResult.h"
#include "Characters/LoLCharacterBase.h"
#include "Characters/LoLChampion.h"
#include "Characters/Data/ChampionData.h"
#include "Components/CooldownComponent.h"
#include "Components/StatComponent.h"

// --- 이즈리얼 수치 상수 (TODO: ChampionDataSubsystem SkillDetailTable 연동 후 제거) ---
namespace EzrealStats
{
	constexpr float Q_Damage_Base    = 35.f;
	constexpr float Q_Damage_ADRatio = 1.3f;
	constexpr float Q_Speed          = 2000.f;
	constexpr float Q_Range          = 1100.f;
	constexpr float Q_Cooldown       = 0.f;
	constexpr float Q_ManaCost       = 28.f;

	constexpr float W_Damage_Base    = 80.f;
	constexpr float W_Damage_APRatio = 0.6f;
	constexpr float W_Speed          = 1600.f;
	constexpr float W_Range          = 1000.f;
	constexpr float W_Cooldown       = 0.f;
	constexpr float W_ManaCost       = 50.f;

	constexpr float E_Blink_Range              = 475.f;
	constexpr float E_Secondary_Damage_Base    = 75.f;
	constexpr float E_Secondary_Damage_ADRatio = 0.5f;
	constexpr float E_Secondary_Speed          = 2000.f;
	constexpr float E_Secondary_Range          = 750.f;
	constexpr float E_Cooldown                 = 0.f;
	constexpr float E_ManaCost                 = 90.f;
}

UEzrealSkillExecutor::UEzrealSkillExecutor()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEzrealSkillExecutor::BeginPlay()
{
	Super::BeginPlay(); // OwnerChar, StatComp, CombatComp, CooldownComp 캐시됨

	OwnerChampion = Cast<ALoLChampion>(GetOwner());
}

void UEzrealSkillExecutor::Execute(ESkillSlot Slot, FVector TargetLoc)
{
	switch (Slot)
	{
	case ESkillSlot::Q: ExecuteQ(TargetLoc); break;
	case ESkillSlot::W: ExecuteW(TargetLoc); break;
	case ESkillSlot::E: ExecuteE(TargetLoc); break;
	case ESkillSlot::R: break; // TODO
	}
}

// ===== Q — 신비한 화살 =====

void UEzrealSkillExecutor::ExecuteQ(FVector TargetLoc)
{
	UChampionData* Data = OwnerChampion ? OwnerChampion->GetChampionData() : nullptr;
	PlayMontage(Data ? Data->QSkillMontage : nullptr);

	if (StatComp)     StatComp->ApplyManaCost(EzrealStats::Q_ManaCost);
	if (CooldownComp) CooldownComp->StartCooldown(TEXT("Skill.Q"), EzrealStats::Q_Cooldown);

	FDamageContext Ctx;
	Ctx.RawDamage        = EzrealStats::Q_Damage_Base
		+ (StatComp ? StatComp->GetAD() * EzrealStats::Q_Damage_ADRatio : 0.f);
	Ctx.DamageType       = EDamageType::Physical;
	Ctx.DamageInstigator = GetOwner();
	Ctx.SourceTag        = TEXT("Ezreal.Q");

	SpawnProjectile((TargetLoc - OwnerChar->GetActorLocation()).GetSafeNormal2D(),
		EzrealStats::Q_Speed, EzrealStats::Q_Range, Ctx, false, true, TEXT("Socket_Q"));
}

// ===== W — 본질 유출 =====

void UEzrealSkillExecutor::ExecuteW(FVector TargetLoc)
{
	UChampionData* Data = OwnerChampion ? OwnerChampion->GetChampionData() : nullptr;
	PlayMontage(Data ? Data->WSkillMontage : nullptr);

	if (StatComp)     StatComp->ApplyManaCost(EzrealStats::W_ManaCost);
	if (CooldownComp) CooldownComp->StartCooldown(TEXT("Skill.W"), EzrealStats::W_Cooldown);

	FDamageContext Ctx;
	Ctx.RawDamage        = EzrealStats::W_Damage_Base
		+ (StatComp ? StatComp->GetAP() * EzrealStats::W_Damage_APRatio : 0.f);
	Ctx.DamageType       = EDamageType::Magical;
	Ctx.DamageInstigator = GetOwner();
	Ctx.SourceTag        = TEXT("Ezreal.W");

	SpawnProjectile((TargetLoc - OwnerChar->GetActorLocation()).GetSafeNormal2D(),
		EzrealStats::W_Speed, EzrealStats::W_Range, Ctx, true, false, TEXT("Socket_Q"));
}

// ===== E — 신비한 이동 =====

void UEzrealSkillExecutor::ExecuteE(FVector TargetLoc)
{
	UChampionData* Data = OwnerChampion ? OwnerChampion->GetChampionData() : nullptr;
	PlayMontage(Data ? Data->ESkillMontage : nullptr);

	if (StatComp)     StatComp->ApplyManaCost(EzrealStats::E_ManaCost);
	if (CooldownComp) CooldownComp->StartCooldown(TEXT("Skill.E"), EzrealStats::E_Cooldown);

	const FVector CurLoc = OwnerChar->GetActorLocation();
	const FVector Dir2D  = (TargetLoc - CurLoc).GetSafeNormal2D();
	const float   Dist   = FMath::Min(FVector::Dist2D(TargetLoc, CurLoc), EzrealStats::E_Blink_Range);

	OwnerChar->TeleportTo(CurLoc + Dir2D * Dist, OwnerChar->GetActorRotation());

	// FireESecondaryShot(); // TODO: 나중에 활성화
}

void UEzrealSkillExecutor::FireESecondaryShot()
{
	constexpr float SearchRadius = 750.f;

	TArray<FOverlapResult> Overlaps;
	OwnerChar->GetWorld()->OverlapMultiByChannel(
		Overlaps, OwnerChar->GetActorLocation(), FQuat::Identity,
		ECC_Pawn, FCollisionShape::MakeSphere(SearchRadius),
		FCollisionQueryParams(NAME_None, false, OwnerChar));

	AActor* NearestEnemy = nullptr;
	float   MinDist      = SearchRadius + 1.f;

	for (const FOverlapResult& R : Overlaps)
	{
		AActor* Other = R.GetActor();
		if (!Other || Other == OwnerChar) continue;

		ALoLCharacterBase* OtherChar = Cast<ALoLCharacterBase>(Other);
		if (!OtherChar || OtherChar->GetTeam() == OwnerChar->GetTeam() || !OtherChar->IsTargetable()) continue;

		const float Dist = FVector::Dist(OwnerChar->GetActorLocation(), Other->GetActorLocation());
		if (Dist < MinDist)
		{
			MinDist      = Dist;
			NearestEnemy = Other;
		}
	}

	if (!NearestEnemy) return;

	FDamageContext Ctx;
	Ctx.RawDamage        = EzrealStats::E_Secondary_Damage_Base
		+ (StatComp ? StatComp->GetAD() * EzrealStats::E_Secondary_Damage_ADRatio : 0.f);
	Ctx.DamageType       = EDamageType::Physical;
	Ctx.DamageInstigator = GetOwner();
	Ctx.SourceTag        = TEXT("Ezreal.E");

	SpawnProjectile(
		(NearestEnemy->GetActorLocation() - OwnerChar->GetActorLocation()).GetSafeNormal2D(),
		EzrealStats::E_Secondary_Speed, EzrealStats::E_Secondary_Range, Ctx, false);
}
