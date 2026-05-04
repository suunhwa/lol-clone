// Fill out your copyright notice in the Description page of Project Settings.

#include "Champions/Ezreal/EzrealSkillExecutor.h"

#include "LeagueofLegends.h"
#include "Engine/OverlapResult.h"
#include "Characters/LoLCharacterBase.h"
#include "Characters/LoLChampion.h"
#include "Characters/Data/ChampionData.h"
#include "Components/CooldownComponent.h"
#include "Components/StatComponent.h"
#include "Manager/ChampionDataSubsystem.h"

UEzrealSkillExecutor::UEzrealSkillExecutor()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEzrealSkillExecutor::BeginPlay()
{
	Super::BeginPlay(); // OwnerChar, StatComp, CombatComp, CooldownComp 캐시됨

	OwnerChampion = Cast<ALoLChampion>(GetOwner());
}

UChampionDataSubsystem* UEzrealSkillExecutor::GetDataSub() const
{
	if (!OwnerChar) { return nullptr; }
	UGameInstance* GI = OwnerChar->GetGameInstance();
	return GI ? GI->GetSubsystem<UChampionDataSubsystem>() : nullptr;
}

FName UEzrealSkillExecutor::GetChampionID() const
{
	if (!OwnerChampion || !OwnerChampion->GetChampionData()) { return NAME_None; }
	return OwnerChampion->GetChampionData()->ChampionID;
}

int32 UEzrealSkillExecutor::GetRank(ESkillSlot Slot) const
{
	if (!OwnerChampion || !OwnerChampion->SkillComp) { return 1; }
	return FMath::Max(1, OwnerChampion->SkillComp->GetRank(Slot));
}

// Factor_Stat1/2 + Coefficient1/2 기반 피해 계산
static float ComputeScaledDamage(const FDetailSkillStatsRow& Row, UStatComponent* Stat)
{
	float Dmg = Row.Base_Value;
	if (!Stat) { return Dmg; }

	auto Apply = [&](const FString& FactorStat, const FString& Coeff)
	{
		if (FactorStat.IsEmpty() || FactorStat.Equals(TEXT("None"), ESearchCase::IgnoreCase)) { return; }
		
		const float C = FCString::Atof(*Coeff);
		
		if (FactorStat.Equals(TEXT("AD"), ESearchCase::IgnoreCase))
		{
			Dmg += Stat->GetAD() * C;
		}
		else if (FactorStat.Equals(TEXT("AP"), ESearchCase::IgnoreCase))
		{
			Dmg += Stat->GetAP() * C;
		}
	};

	Apply(Row.Factor_Stat1, Row.Coefficient1);
	Apply(Row.Factor_Stat2, Row.Coefficient2);
	return Dmg;
}

void UEzrealSkillExecutor::Execute(ESkillSlot Slot, FVector TargetLoc)
{
	switch (Slot)
	{
	case ESkillSlot::Q: ExecuteQ(TargetLoc);
		break;
	case ESkillSlot::W: ExecuteW(TargetLoc);
		break;
	case ESkillSlot::E: ExecuteE(TargetLoc);
		break;
	case ESkillSlot::R: break; // TODO
	}
}

// Q
void UEzrealSkillExecutor::ExecuteQ(FVector TargetLoc)
{
	UChampionData* Data = OwnerChampion ? OwnerChampion->GetChampionData() : nullptr;
	PlayMontage(Data ? Data->QSkillMontage : nullptr);

	UChampionDataSubsystem* Sub = GetDataSub();
	const FDetailSkillStatsRow* Stats = Sub
		                                    ? Sub->GetSkillStats(GetChampionID(), TEXT("Q"), GetRank(ESkillSlot::Q))
		                                    : nullptr;

	const float ManaCost = Stats ? Stats->Cost : 28.f;
	const float Cooldown = Stats ? Stats->CoolDown : 5.5f;

	if (Stats)
	{
		PRINTLOG_SH(
			TEXT("[Q] 테이블 ─ Rank:%d Base:%.1f FactorStat1:%s Coeff1:%s FactorStat2:%s Coeff2:%s Cost:%.1f CD:%.2f"),
			GetRank(ESkillSlot::Q),
			Stats->Base_Value, *Stats->Factor_Stat1, *Stats->Coefficient1,
			*Stats->Factor_Stat2, *Stats->Coefficient2,
			Stats->Cost, Stats->CoolDown);
	}
	else
	{
		PRINTLOG_SH(TEXT("[Q] 테이블 로드 실패 — fallback 수치 사용"));
	}

	if (StatComp)
	{
		StatComp->ApplyManaCost(ManaCost);
	}
	
	if (CooldownComp)
	{
		CooldownComp->StartCooldown(TEXT("Skill.Q"), Cooldown);
	}

	FDamageContext Ctx;
	Ctx.RawDamage = Stats ? ComputeScaledDamage(*Stats, StatComp) : 20.f;

	PRINTLOG_SH(TEXT("[Q] 계산 ─ AD:%.1f  최종데미지:%.1f  마나차감:%.1f  쿨타임:%.2f"),
	            StatComp ? StatComp->GetAD() : 0.f, Ctx.RawDamage, ManaCost, Cooldown);
	Ctx.DamageType = EDamageType::Physical;
	Ctx.DamageInstigator = GetOwner();
	Ctx.SourceTag = TEXT("Ezreal.Q");

	SpawnProjectile((TargetLoc - OwnerChar->GetActorLocation()).GetSafeNormal2D(),
	                2000.f, 1100.f, Ctx, false, true, TEXT("Socket_Q"));
}

// W
void UEzrealSkillExecutor::ExecuteW(FVector TargetLoc)
{
	UChampionData* Data = OwnerChampion ? OwnerChampion->GetChampionData() : nullptr;
	PlayMontage(Data ? Data->WSkillMontage : nullptr);

	UChampionDataSubsystem* Sub = GetDataSub();
	const FDetailSkillStatsRow* Stats = Sub
		                                    ? Sub->GetSkillStats(GetChampionID(), TEXT("W"), GetRank(ESkillSlot::W))
		                                    : nullptr;

	const float ManaCost = Stats ? Stats->Cost : 50.f;
	const float Cooldown = Stats ? Stats->CoolDown : 12.f;

	if (Stats)
	{
		PRINTLOG_SH(
			TEXT("[W] 테이블 ─ Rank:%d Base:%.1f FactorStat1:%s Coeff1:%s FactorStat2:%s Coeff2:%s Cost:%.1f CD:%.2f"),
			GetRank(ESkillSlot::W),
			Stats->Base_Value, *Stats->Factor_Stat1, *Stats->Coefficient1,
			*Stats->Factor_Stat2, *Stats->Coefficient2,
			Stats->Cost, Stats->CoolDown);
	}
	else
	{
		PRINTLOG_SH(TEXT("[W] 테이블 로드 실패 — fallback 수치 사용"));
	}

	if (StatComp)
	{
		StatComp->ApplyManaCost(ManaCost);
	}
	if (CooldownComp)
	{
		CooldownComp->StartCooldown(TEXT("Skill.W"), Cooldown);
	}

	FDamageContext Ctx;
	Ctx.RawDamage = Stats ? ComputeScaledDamage(*Stats, StatComp) : 80.f;

	PRINTLOG_SH(TEXT("[W] 계산 ─ AD:%.1f AP:%.1f  최종데미지:%.1f  마나차감:%.1f  쿨타임:%.2f"),
	            StatComp ? StatComp->GetAD() : 0.f, StatComp ? StatComp->GetAP() : 0.f,
	            Ctx.RawDamage, ManaCost, Cooldown);
	Ctx.DamageType = EDamageType::Magical;
	Ctx.DamageInstigator = GetOwner();
	Ctx.SourceTag = TEXT("Ezreal.W");

	SpawnProjectile((TargetLoc - OwnerChar->GetActorLocation()).GetSafeNormal2D(),
	                1600.f, 1000.f, Ctx, true, false, TEXT("Socket_Q"));
}

// E
void UEzrealSkillExecutor::ExecuteE(FVector TargetLoc)
{
	UChampionData* Data = OwnerChampion ? OwnerChampion->GetChampionData() : nullptr;
	PlayMontage(Data ? Data->ESkillMontage : nullptr);

	UChampionDataSubsystem* Sub = GetDataSub();
	const FDetailSkillStatsRow* Stats = Sub
		                                    ? Sub->GetSkillStats(GetChampionID(), TEXT("E"), GetRank(ESkillSlot::E))
		                                    : nullptr;
	const FSkillMechanicsRow* Mech = Sub ? Sub->GetSkillMechanics(GetChampionID(), TEXT("E")) : nullptr;

	const float ManaCost = Stats ? Stats->Cost : 90.f;
	const float Cooldown = Stats ? Stats->CoolDown : 11.f;
	const float BlinkRange = Mech ? Mech->Param1_Value : 475.f;

	if (Stats)
	{
		PRINTLOG_SH(TEXT("[E] 테이블 ─ Rank:%d Base:%.1f Cost:%.1f CD:%.2f"),
		            GetRank(ESkillSlot::E), Stats->Base_Value, Stats->Cost, Stats->CoolDown);
	}
	else
	{
		PRINTLOG_SH(TEXT("[E] 테이블 로드 실패 — fallback 수치 사용"));
	}

	PRINTLOG_SH(TEXT("[E] 계산 ─ BlinkRange:%.1f (%s)  마나차감:%.1f  쿨타임:%.2f"),
	            BlinkRange, Mech ? TEXT("테이블") : TEXT("fallback"), ManaCost, Cooldown);

	if (StatComp)
	{
		StatComp->ApplyManaCost(ManaCost);
	}
	
	if (CooldownComp)
	{
		CooldownComp->StartCooldown(TEXT("Skill.E"), Cooldown);
	}

	const FVector CurLoc = OwnerChar->GetActorLocation();
	const FVector Dir2D = (TargetLoc - CurLoc).GetSafeNormal2D();
	const float Dist = FMath::Min(FVector::Dist2D(TargetLoc, CurLoc), BlinkRange);

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
	float MinDist = SearchRadius + 1.f;

	for (const FOverlapResult& R : Overlaps)
	{
		AActor* Other = R.GetActor();
		if (!Other || Other == OwnerChar) { continue; }

		ALoLCharacterBase* OtherChar = Cast<ALoLCharacterBase>(Other);
		if (!OtherChar || OtherChar->GetTeam() == OwnerChar->GetTeam() || !OtherChar->IsTargetable()) { continue; }

		const float Dist = FVector::Dist(OwnerChar->GetActorLocation(), Other->GetActorLocation());
		if (Dist < MinDist)
		{
			MinDist = Dist;
			NearestEnemy = Other;
		}
	}

	if (!NearestEnemy) { return; }

	UChampionDataSubsystem* Sub = GetDataSub();
	const FDetailSkillStatsRow* Stats = Sub
		                                    ? Sub->GetSkillStats(GetChampionID(), TEXT("E"), GetRank(ESkillSlot::E))
		                                    : nullptr;

	FDamageContext Ctx;
	Ctx.RawDamage = Stats ? ComputeScaledDamage(*Stats, StatComp) : 80.f;
	Ctx.DamageType = EDamageType::Physical;
	Ctx.DamageInstigator = GetOwner();
	Ctx.SourceTag = TEXT("Ezreal.E");

	SpawnProjectile(
		(NearestEnemy->GetActorLocation() - OwnerChar->GetActorLocation()).GetSafeNormal2D(),
		2000.f, 750.f, Ctx, false);
}
