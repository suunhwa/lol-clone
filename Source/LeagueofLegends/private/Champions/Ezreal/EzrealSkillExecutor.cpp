// Fill out your copyright notice in the Description page of Project Settings.

#include "Champions/Ezreal/EzrealSkillExecutor.h"

#include "LeagueofLegends.h"
#include "DrawDebugHelpers.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Champions/Projectile/ChampionSkillProjectile.h"
#include "Engine/OverlapResult.h"
#include "Characters/LoLCharacterBase.h"
#include "Characters/LoLChampion.h"
#include "Characters/Data/ChampionData.h"
#include "Components/CooldownComponent.h"
#include "Components/StatComponent.h"
#include "Kismet/GameplayStatics.h"
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
	if (!OwnerChampion || !OwnerChampion->GetChampionData())
	{
		return NAME_None;
	}
	
	return OwnerChampion->GetChampionData()->ChampionID;
}

int32 UEzrealSkillExecutor::GetRank(ESkillSlot Slot) const
{
	if (!OwnerChampion || !OwnerChampion->SkillComp)
	{
		return 1;
	}
	
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
	case ESkillSlot::R: ExecuteR(TargetLoc);
		break;
	}
}

// Q
void UEzrealSkillExecutor::ExecuteQ(FVector TargetLoc)
{
	UChampionData* Data = OwnerChampion ? OwnerChampion->GetChampionData() : nullptr;

	if (Data && Data->QSkillMontage)
	{
		OwnerChar->Multicast_PlayMontage(Data->QSkillMontage);
	}

	UChampionDataSubsystem* Sub = GetDataSub();
	const FDetailSkillStatsRow* Stats = Sub
		                                    ? Sub->GetSkillStats(GetChampionID(), TEXT("Q"), GetRank(ESkillSlot::Q))
		                                    : nullptr;

	const float ManaCost = Stats ? Stats->Cost : 28.f;
	const float Cooldown = Stats ? Stats->CoolDown : 5.5f;

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
	Ctx.DamageType = EDamageType::Physical;
	Ctx.DamageInstigator = GetOwner();
	Ctx.SourceTag = TEXT("Ezreal.Q");

	AChampionSkillProjectile* Proj = SpawnProjectile((TargetLoc - OwnerChar->GetActorLocation()).GetSafeNormal2D(),
	                                                 2000.f,
	                                                 1100.f,
	                                                 Ctx,
	                                                 false,
	                                                 true,
	                                                 TEXT("Socket_Q")
	                                                 );

	if (Proj && Q_MuzzleEffect)
	{
		UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			Q_MuzzleEffect,
			Proj->GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			false); // bAutoDestroy = false, 발사체 수명에 맞춰 같이 사라짐
		
		if (NiagaraComp)
		{
			NiagaraComp->SetWorldScale3D(FVector(0.4f));
		}
	}
	
	if (Q_CastSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(), Q_CastSound, OwnerChar->GetActorLocation());
	}
	
	/* TODO: 몽타주에 ExitRun/ExitIdle 섹션 추가 후 활성화
	bool bMoving = OwnerChar->GetVelocity().SizeSquared2D() > 100.f;
	FName ExitSection = bMoving ? TEXT("ExitRun") : TEXT("ExitIdle");
	FTimerHandle ExitTimer;
	OwnerChar->GetWorldTimerManager().SetTimer(ExitTimer, [this, Data, ExitSection]()
	{
		if (!OwnerChar || !Data || !Data->QSkillMontage) { return; }
		UAnimInstance* Anim = OwnerChar->GetMesh()->GetAnimInstance();
		if (Anim) { Anim->Montage_JumpToSection(ExitSection, Data->QSkillMontage); }
	}, 0.4f, false);
	*/
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
			Stats->Base_Value,
			*Stats->Factor_Stat1,
			*Stats->Coefficient1,
			*Stats->Factor_Stat2,
			*Stats->Coefficient2,
			Stats->Cost,
			Stats->CoolDown);
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
	            StatComp ? StatComp->GetAD() : 0.f,
	            StatComp ? StatComp->GetAP() : 0.f,
	            Ctx.RawDamage,
	            ManaCost,
	            Cooldown);
	Ctx.DamageType = EDamageType::Magical;
	Ctx.DamageInstigator = GetOwner();
	Ctx.SourceTag = TEXT("Ezreal.W");

	AChampionSkillProjectile* WProj = SpawnProjectile(
		(TargetLoc - OwnerChar->GetActorLocation()).GetSafeNormal2D(),
		1600.f,
		1000.f,
		Ctx,
		true,
		false,
		TEXT("Socket_Q"));

	if (WProj && W_MuzzleEffect)
	{
		if (UNiagaraComponent* WFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
			W_MuzzleEffect,
			WProj->GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			false))
		{
			WFX->SetWorldScale3D(FVector(0.5f));
		}
	}

	if (W_CastSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(), W_CastSound, OwnerChar->GetActorLocation());
	}
}

// E
void UEzrealSkillExecutor::ExecuteE(FVector TargetLoc)
{
	UChampionData* Data = OwnerChampion ? OwnerChampion->GetChampionData() : nullptr;

	/* TODO: 몽타주에 방향별 섹션(E_0/90/180/-90/-180) 추가 후 활성화
	FVector Forward = OwnerChar->GetActorForwardVector();
	FVector BlinkDir = (TargetLoc - OwnerChar->GetActorLocation()).GetSafeNormal2D();
	float Dot = FVector::DotProduct(Forward, BlinkDir);
	float Cross = Forward.X * BlinkDir.Y - Forward.Y * BlinkDir.X;
	float Angle = FMath::RadiansToDegrees(FMath::Atan2(Cross, Dot));
	float Abs = FMath::Abs(Angle);

	FName Section;
	if (Abs < 45.f) { Section = TEXT("E_0"); }
	else if (Abs > 135.f) { Section = (Angle > 0.f) ? TEXT("E_180") : TEXT("E_-180"); }
	else if (Angle > 0.f) { Section = TEXT("E_90"); }
	else { Section = TEXT("E_-90"); }
	*/

	if (Data && Data->ESkillMontage)
	{
		OwnerChar->Multicast_PlayMontage(Data->ESkillMontage);
	}

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
		            GetRank(ESkillSlot::E),
		            Stats->Base_Value,
		            Stats->Cost,
		            Stats->CoolDown);
	}
	else
	{
		PRINTLOG_SH(TEXT("[E] 테이블 로드 실패 — fallback 수치 사용"));
	}

	PRINTLOG_SH(TEXT("[E] 계산 ─ BlinkRange:%.1f (%s)  마나차감:%.1f  쿨타임:%.2f"),
	            BlinkRange,
	            Mech ? TEXT("테이블") : TEXT("fallback"),
	            ManaCost,
	            Cooldown);

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
	const FVector ArriveLoc = CurLoc + Dir2D * Dist;

	// 출발지 이펙트
	if (E_DepartEffect)
	{
		if (UNiagaraComponent* DepartFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), E_DepartEffect, CurLoc, FRotator::ZeroRotator))
		{
			DepartFX->SetWorldScale3D(FVector(0.5f));
		}
	}

	if (E_BlinkSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(), E_BlinkSound, CurLoc);
	}

	OwnerChar->TeleportTo(ArriveLoc, OwnerChar->GetActorRotation());

	// 도착지 이펙트
	if (E_ArriveEffect)
	{
		if (UNiagaraComponent* ArriveFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), E_ArriveEffect, ArriveLoc, FRotator::ZeroRotator))
		{
			ArriveFX->SetWorldScale3D(FVector(0.5f));
		}
	}

	/* TODO: 몽타주에 ExitRun/ExitIdle 섹션 추가 후 활성화
	bool bMoving = OwnerChar->GetVelocity().SizeSquared2D() > 100.f;
	FName ExitSection = bMoving ? TEXT("ExitRun") : TEXT("ExitIdle");
	FTimerHandle ExitTimer;
	OwnerChar->GetWorldTimerManager().SetTimer(ExitTimer, [this, Data, ExitSection]()
	{
		if (!OwnerChar || !Data || !Data->ESkillMontage) { return; }
		UAnimInstance* Anim = OwnerChar->GetMesh()->GetAnimInstance();
		if (Anim) { Anim->Montage_JumpToSection(ExitSection, Data->ESkillMontage); }
	}, 0.3f, false);
	*/

	FireESecondaryShot();
}

// R
void UEzrealSkillExecutor::ExecuteR(FVector TargetLoc)
{
	UChampionData* Data = OwnerChampion ? OwnerChampion->GetChampionData() : nullptr;
	PlayMontage(Data ? Data->RSkillMontage : nullptr);

	UChampionDataSubsystem* Sub = GetDataSub();
	const FDetailSkillStatsRow* Stats = Sub
		                                    ? Sub->GetSkillStats(GetChampionID(), TEXT("R"), GetRank(ESkillSlot::R))
		                                    : nullptr;

	const float ManaCost = Stats ? Stats->Cost : 100.f;
	const float Cooldown = Stats ? Stats->CoolDown : 120.f;

	if (StatComp)
	{
		StatComp->ApplyManaCost(ManaCost);
	}

	if (CooldownComp)
	{
		CooldownComp->StartCooldown(TEXT("Skill.R"), Cooldown);
	}

	const FVector Origin = OwnerChar->GetActorLocation();
	const FVector Dir = (TargetLoc - Origin).GetSafeNormal2D();
	constexpr float RLength = 40000.f; // 맵 끝까지
	constexpr float RHalfWidth = 160.f;
	constexpr float VisualDuration = 1.2f;

	const FVector Right = FVector::CrossProduct(Dir, FVector::UpVector) * RHalfWidth;
	UWorld* World = OwnerChar->GetWorld();

	// 중앙선 + 좌우 외곽선만
	/*DrawDebugLine(World, Origin, Origin + Dir * RLength, FColor::Blue, false, VisualDuration, 0, 20.f);
	DrawDebugLine(World, Origin + Right, Origin + Right + Dir * RLength, FColor::Cyan, false, VisualDuration, 0, 6.f);
	DrawDebugLine(World, Origin - Right, Origin - Right + Dir * RLength, FColor::Cyan, false, VisualDuration, 0, 6.f);
	*/

	// 데미지 컨텍스트
	FDamageContext Ctx;
	Ctx.RawDamage = Stats ? ComputeScaledDamage(*Stats, StatComp) : 350.f;
	Ctx.DamageType = EDamageType::Magical;
	Ctx.DamageInstigator = GetOwner();
	Ctx.SourceTag = TEXT("Ezreal.R");

	PRINTLOG_SH(TEXT("[R] Charge ─ 데미지:%.1f"), Ctx.RawDamage);

	if (R_CastSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(), R_CastSound, OwnerChar->GetActorLocation());
	}

	// 애니메이션 차지 후 발사 (1초 딜레이)
	FTimerHandle RTimer;
	OwnerChar->GetWorldTimerManager().SetTimer(RTimer,
	                                           [this, Dir, Ctx]()
	                                           {
		                                           if (!OwnerChar) return;
		                                           PRINTLOG_SH(TEXT("[R] 발사!"));
		                                           if (AChampionSkillProjectile* Proj = SpawnProjectile(
			                                           Dir,
			                                           2000.f,
			                                           40000.f,
			                                           Ctx,
			                                           true,
			                                           false,
			                                           TEXT("Socket_Q")))
			                                           {
			                                           	Proj->DebugTrailHalfWidth = 160.f;

			                                           	if (R_MuzzleEffect)
			                                           	{
			                                           		if (UNiagaraComponent* RFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
			                                           			R_MuzzleEffect,
			                                           			Proj->GetRootComponent(),
			                                           			NAME_None,
			                                           			FVector::ZeroVector,
			                                           			FRotator::ZeroRotator,
			                                           			EAttachLocation::SnapToTarget,
			                                           			false))
			                                           		{
			                                           			RFX->SetWorldScale3D(FVector(0.4f));
			                                           		}
			                                           	}
			                                           }
	                                           },
	                                           1.0f,
	                                           false);
}

// TODO: E 스킬 후 평타 (자동, 범위 내에 가장 가까운 적. 범위는 하드코딩)
void UEzrealSkillExecutor::FireESecondaryShot()
{
	constexpr float SearchRadius = 750.f;

	TArray<FOverlapResult> Overlaps;
	OwnerChar->GetWorld()->OverlapMultiByChannel(
		Overlaps,
		OwnerChar->GetActorLocation(),
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(SearchRadius),
		FCollisionQueryParams(NAME_None, false, OwnerChar));

	AActor* NearestEnemy = nullptr;
	float MinDist = SearchRadius + 1.f;

	for (const FOverlapResult& R : Overlaps)
	{
		AActor* Other = R.GetActor();
		if (!Other || Other == OwnerChar) { continue; }

		ALoLCharacterBase* OtherChar = Cast<ALoLCharacterBase>(Other);
		if (!OtherChar || 
			ITargetable::Execute_GetTeam(OtherChar) == OwnerChar->GetTeam_Implementation() || 
			!ITargetable::Execute_IsTargetable(OtherChar))
		{
			continue;
		}

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
		2000.f,
		750.f,
		Ctx,
		false);
}
