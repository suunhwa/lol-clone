// Fill out your copyright notice in the Description page of Project Settings.

#include "Champions/Ezreal/EzrealSkillExecutor.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

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
#include "Interfaces/Targetable.h"
#include "Kismet/GameplayStatics.h"
#include "Manager/ChampionDataSubsystem.h"
#include "Type/RiftTypes.h"

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
	UChampionDataSubsystem* Sub = GetDataSub();
	const FDetailSkillStatsRow* Stats = Sub
		                                    ? Sub->GetSkillStats(GetChampionID(), TEXT("Q"), GetRank(ESkillSlot::Q))
		                                    : nullptr;

	const float ManaCost = Stats ? Stats->Cost : 28.f;
	const float Cooldown = Stats ? Stats->CoolDown : 5.5f;

	if (StatComp && StatComp->GetCurrentMana() < ManaCost)
	{
		PRINTLOG_SH(TEXT("[Q] 마나 부족 (현재:%.0f 필요:%.0f)"), StatComp->GetCurrentMana(), ManaCost);
		return;
	}

	UChampionData* Data = OwnerChampion ? OwnerChampion->GetChampionData() : nullptr;

	if (Data && Data->QSkillMontage)
	{
		OwnerChar->Multicast_PlayMontage(Data->QSkillMontage);
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

	// Q는 타워/구조물에 피해를 주지 않음
	if (Proj) { Proj->bCanDamageStructures = false; }

	// W 마크 대상 적중 시 마크 소비 (실제 롤과 동일)
	if (Proj && WMarkTarget.IsValid())
	{
		Proj->OnHitDelegate.BindUObject(this, &UEzrealSkillExecutor::OnWMarkConsumed);
	}

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

// W — 자체 데미지 없음. 챔피언/타워 적중 시 고리(마크) 적용. 이후 평타로 마크 소비 시 실제 데미지
void UEzrealSkillExecutor::ExecuteW(FVector TargetLoc)
{
	UChampionDataSubsystem* Sub = GetDataSub();
	const FDetailSkillStatsRow* Stats = Sub
		                                    ? Sub->GetSkillStats(GetChampionID(), TEXT("W"), GetRank(ESkillSlot::W))
		                                    : nullptr;

	const float ManaCost = Stats ? Stats->Cost : 50.f;
	const float Cooldown = Stats ? Stats->CoolDown : 12.f;
	const float BonusDmg = Stats ? ComputeScaledDamage(*Stats, StatComp) : 80.f;

	if (StatComp && StatComp->GetCurrentMana() < ManaCost)
	{
		PRINTLOG_SH(TEXT("[W] 마나 부족 (현재:%.0f 필요:%.0f)"), StatComp->GetCurrentMana(), ManaCost);
		return;
	}

	UChampionData* Data = OwnerChampion ? OwnerChampion->GetChampionData() : nullptr;
	PlayMontage(Data ? Data->WSkillMontage : nullptr);

	if (StatComp) { StatComp->ApplyManaCost(ManaCost); }
	if (CooldownComp) { CooldownComp->StartCooldown(TEXT("Skill.W"), Cooldown); }

	// W 발사체 자체는 데미지 0 — 피격 콜백에서 마크 적용
	FDamageContext Ctx;
	Ctx.RawDamage = 0.f;
	Ctx.DamageType = EDamageType::Magical;
	Ctx.DamageInstigator = GetOwner();
	Ctx.SourceTag = TEXT("Ezreal.W");

	AChampionSkillProjectile* WProj = SpawnProjectile(
		(TargetLoc - OwnerChar->GetActorLocation()).GetSafeNormal2D(),
		1600.f, 1000.f, Ctx, true, false, TEXT("Socket_Q"));

	if (WProj)
	{
		WProjectile = WProj;
		const float CapturedBonusDmg = BonusDmg;
		WProj->OnHitDelegate.BindUObject(this, &UEzrealSkillExecutor::OnWProjectileHit);
		WMarkBonusDamage = CapturedBonusDmg;

		if (W_MuzzleEffect)
		{
			if (UNiagaraComponent* WFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
				W_MuzzleEffect, WProj->GetRootComponent(), NAME_None,
				FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, false))
			{
				WFX->SetWorldScale3D(FVector(0.5f));
			}
		}
	}

	if (W_CastSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), W_CastSound, OwnerChar->GetActorLocation());
	}
}

void UEzrealSkillExecutor::SetWMark(AActor* Target, float BonusDamage)
{
	WMarkTarget = Target;
	WMarkBonusDamage = BonusDamage;

	// 이전 마크 이펙트 정리 (중복 방지)
	if (WMarkEffectComp)
	{
		WMarkEffectComp->DestroyComponent();
		WMarkEffectComp = nullptr;
	}

	// 대상 몸에 이펙트 부착
	if (W_MarkEffect && Target)
	{
		USceneComponent* AttachTarget = nullptr;

		// 챔피언/미니언: 스켈레탈 메시에 붙여야 몸을 따라다님
		if (USkeletalMeshComponent* Mesh = Target->FindComponentByClass<USkeletalMeshComponent>())
		{
			AttachTarget = Mesh;
		}
		else
		{
			AttachTarget = Target->GetRootComponent();
		}

		if (AttachTarget)
		{
			float TargetRadius = 42.f;
			FVector EffectWorldLoc = Target->GetActorLocation();

			if (UCapsuleComponent* Capsule = Target->FindComponentByClass<UCapsuleComponent>())
			{
				TargetRadius = Capsule->GetScaledCapsuleRadius();
				EffectWorldLoc = Capsule->GetComponentLocation();
				EffectWorldLoc.Z += W_MarkEffectZOffset_Champion;

				PRINTLOG_SH(TEXT("[W Mark Champ] CapsuleLoc.Z=%.1f HalfH=%.1f FinalZ=%.1f"),
				            Capsule->GetComponentLocation().Z,
				            Capsule->GetScaledCapsuleHalfHeight(),
				            EffectWorldLoc.Z);
			}
			else
			{
				// 타워: GetActorBounds의 Origin이 바운딩박스 중심
				FVector Origin, Extent;
				Target->GetActorBounds(true, Origin, Extent);
				TargetRadius = FMath::Max(Extent.X, Extent.Y);
				EffectWorldLoc = Origin;
				EffectWorldLoc.Z += W_MarkEffectZOffset_Tower;

				PRINTLOG_SH(TEXT("[W Mark] OriginZ=%.1f ExtentZ=%.1f FinalZ=%.1f"),
				            Origin.Z, Extent.Z, EffectWorldLoc.Z);
			}

			WMarkEffectComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
				W_MarkEffect,
				AttachTarget,
				NAME_None,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::SnapToTarget,
				true);

			if (WMarkEffectComp)
			{
				const float Scale = (TargetRadius / 42.f) * W_MarkEffectScale;
				WMarkEffectComp->SetWorldScale3D(FVector(Scale));
				WMarkEffectComp->SetWorldLocation(EffectWorldLoc);

				PRINTLOG_SH(TEXT("[W Mark] SetWorldLocation 요청=%.1f / 실제 컴포넌트Z=%.1f"),
				            EffectWorldLoc.Z,
				            WMarkEffectComp->GetComponentLocation().Z);
			}
		}
	}

	// 마크 지속 4초 후 자동 소멸
	if (OwnerChar)
	{
		OwnerChar->GetWorldTimerManager().SetTimer(WMarkExpireTimer, this,
			&UEzrealSkillExecutor::ClearWMark, 4.f, false);
	}
}

void UEzrealSkillExecutor::ClearWMark()
{
	WMarkTarget = nullptr;
	WMarkBonusDamage = 0.f;
	if (OwnerChar)
	{
		OwnerChar->GetWorldTimerManager().ClearTimer(WMarkExpireTimer);
	}

	if (WMarkEffectComp)
	{
		WMarkEffectComp->DestroyComponent();
		WMarkEffectComp = nullptr;
	}
}

// W 발사체가 적에게 적중했을 때 — 챔피언/타워에만 마크 적용 후 발사체 소멸, 미니언은 무시하고 통과
void UEzrealSkillExecutor::OnWProjectileHit(AActor* Target)
{
	if (!Target || !Target->GetClass()->ImplementsInterface(UTargetable::StaticClass())) { return; }

	EUnitType Type = ITargetable::Execute_GetUnitType(Target);
	if (Type != EUnitType::Champion && Type != EUnitType::Tower) { return; }

	SetWMark(Target, WMarkBonusDamage);
	PRINTLOG_SH(TEXT("[W] 고리 적용 → %s (보너스딜: %.1f)"), *GetNameSafe(Target), WMarkBonusDamage);

	// 챔피언/타워 적중 시 발사체 소멸 (마크만 남김)
	if (WProjectile.IsValid())
	{
		WProjectile->Destroy();
		WProjectile = nullptr;
	}
}

// 마크가 걸린 적을 평타/E 미사일로 맞췄을 때 보너스 데미지 적용
void UEzrealSkillExecutor::OnWMarkConsumed(AActor* Target)
{
	if (!WMarkTarget.IsValid() || WMarkTarget.Get() != Target) { return; }

	FDamageContext Ctx;
	Ctx.RawDamage = WMarkBonusDamage;
	Ctx.DamageType = EDamageType::Magical;
	Ctx.DamageInstigator = GetOwner();
	Ctx.SourceTag = TEXT("Ezreal.W.Mark");

	if (CombatComp)
	{
		CombatComp->DealDamage(Target, Ctx);
	}

	PRINTLOG_SH(TEXT("[W] 고리 소비 → %s 보너스딜: %.1f"), *GetNameSafe(Target), WMarkBonusDamage);
	ClearWMark();
}

// 평타 발사체가 스폰된 직후 호출 — 마크 타겟이면 피격 콜백으로 마크 소비 연결
void UEzrealSkillExecutor::OnBasicAttackFired(AChampionSkillProjectile* Proj, AActor* Target)
{
	if (!Proj || !WMarkTarget.IsValid() || WMarkTarget.Get() != Target) { return; }

	EUnitType Type = ITargetable::Execute_GetUnitType(Target);
	if (Type != EUnitType::Champion && Type != EUnitType::Tower) { return; }

	Proj->OnHitDelegate.BindUObject(this, &UEzrealSkillExecutor::OnWMarkConsumed);
}

// E
void UEzrealSkillExecutor::ExecuteE(FVector TargetLoc)
{
	UChampionDataSubsystem* Sub = GetDataSub();
	const FDetailSkillStatsRow* Stats = Sub
		                                    ? Sub->GetSkillStats(GetChampionID(), TEXT("E"), GetRank(ESkillSlot::E))
		                                    : nullptr;
	const FSkillMechanicsRow* Mech = Sub ? Sub->GetSkillMechanics(GetChampionID(), TEXT("E")) : nullptr;

	const float ManaCost = Stats ? Stats->Cost : 90.f;
	const float Cooldown = Stats ? Stats->CoolDown : 11.f;
	const float BlinkRange = Mech ? Mech->Param1_Value : 475.f;

	if (StatComp && StatComp->GetCurrentMana() < ManaCost)
	{
		PRINTLOG_SH(TEXT("[E] 마나 부족 (현재:%.0f 필요:%.0f)"), StatComp->GetCurrentMana(), ManaCost);
		return;
	}

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

	// 블링크 직후 이동 정지
	if (AController* Ctrl = OwnerChar->GetController())
	{
		Ctrl->StopMovement();
	}

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
	UChampionDataSubsystem* Sub = GetDataSub();
	const FDetailSkillStatsRow* Stats = Sub
		                                    ? Sub->GetSkillStats(GetChampionID(), TEXT("R"), GetRank(ESkillSlot::R))
		                                    : nullptr;

	const float ManaCost = Stats ? Stats->Cost : 100.f;
	const float Cooldown = Stats ? Stats->CoolDown : 120.f;

	if (StatComp && StatComp->GetCurrentMana() < ManaCost)
	{
		PRINTLOG_SH(TEXT("[R] 마나 부족 (현재:%.0f 필요:%.0f)"), StatComp->GetCurrentMana(), ManaCost);
		return;
	}

	UChampionData* Data = OwnerChampion ? OwnerChampion->GetChampionData() : nullptr;
	PlayMontage(Data ? Data->RSkillMontage : nullptr);

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

	// 장전 중 이동 차단
	if (UCharacterMovementComponent* MoveComp = OwnerChar->GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->MaxWalkSpeed = 0.f;
	}

	// 애니메이션 차지 후 발사 (1초 딜레이)
	FTimerHandle RTimer;
	OwnerChar->GetWorldTimerManager().SetTimer(RTimer,
	                                           [this, Dir, Ctx]()
	                                           {
		                                           if (!OwnerChar) return;

		                                           // 이동 복구
		                                           if (UCharacterMovementComponent* MoveComp = OwnerChar->GetCharacterMovement())
		                                           {
			                                           MoveComp->MaxWalkSpeed = StatComp ? StatComp->GetMoveSpeed() : 350.f;
		                                           }

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
			                                           	Proj->SetCollisionRadius(160.f);

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

// E 블링크 후 도착지 주변 챔피언에게만 유도 미사일 발사 (미니언 제외)
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

		if (!Other->GetClass()->ImplementsInterface(UTargetable::StaticClass())) { continue; }
		if (ITargetable::Execute_GetTeam(Other) == OwnerChar->GetTeam_Implementation()) { continue; }
		if (!ITargetable::Execute_IsTargetable(Other)) { continue; }

		// 챔피언에게만 유도 — 미니언/구조물 제외
		if (ITargetable::Execute_GetUnitType(Other) != EUnitType::Champion) { continue; }

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

	AChampionSkillProjectile* EProj = SpawnProjectile(
		(NearestEnemy->GetActorLocation() - OwnerChar->GetActorLocation()).GetSafeNormal2D(),
		2000.f, 750.f, Ctx, false);

	// E 미사일이 W 마크 챔피언을 맞추면 마크 소비
	if (EProj && WMarkTarget.IsValid() && WMarkTarget.Get() == NearestEnemy)
	{
		EProj->OnHitDelegate.BindUObject(this, &UEzrealSkillExecutor::OnWMarkConsumed);
	}
}
