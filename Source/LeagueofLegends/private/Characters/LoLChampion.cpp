// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/LoLChampion.h"

#include "LeagueofLegends.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Characters/Data/ChampionData.h"
#include "Champions/Projectile/ChampionSkillProjectile.h"
#include "Components/CombatComponent.h"
#include "Components/StatComponent.h"
#include "Components/StateComponent.h"
#include "Components/TagComponent.h"
#include "Components/SkillComponent.h"
#include "Components/SkillExecutorComponent.h"
#include "Components/TargetingComponent.h"
#include "Manager/ChampionDataSubsystem.h"
#include "Net/UnrealNetwork.h"

ALoLChampion::ALoLChampion()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ALoLChampion::BeginPlay()
{
	Super::BeginPlay();

	if (SkillComp)
	{
		SkillComp->OnSkillActivated.AddUObject(this, &ALoLChampion::HandleSkillActivated);

		// 테스트용: 모든 스킬 랭크 1로 설정
		// TODO: 레벨업 시 플레이어가 직접 할당하도록 변경 예정
		SkillComp->AssignSkillPoint(ESkillSlot::Q);
		SkillComp->AssignSkillPoint(ESkillSlot::W);
		SkillComp->AssignSkillPoint(ESkillSlot::E);
		SkillComp->AssignSkillPoint(ESkillSlot::R);
	}

	if (!ChampionData) { return; }

	UChampionDataSubsystem* Sub = GetGameInstance()->GetSubsystem<UChampionDataSubsystem>();
	if (!Sub) { return; }

	Sub->ApplyVisuals(this, ChampionData);

	if (HasAuthority())
	{
		Sub->ApplyStats(this, ChampionData);
		CreateSkillExecutor();
	}
}

void ALoLChampion::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALoLChampion, ChampionData);
}

void ALoLChampion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALoLChampion::OnRep_ChampionData()
{
	UChampionDataSubsystem* Sub = GetGameInstance()->GetSubsystem<UChampionDataSubsystem>();
	if (Sub && ChampionData)
	{
		Sub->ApplyVisuals(this, ChampionData);
	}
}

// ChampionData 세팅 (런타임, 캐릭터 선택 후) 
void ALoLChampion::SetChampionData(UChampionData* Data)
{
	if (!HasAuthority() || !Data) { return; }

	ChampionData = Data;

	UChampionDataSubsystem* Sub = GetGameInstance()->GetSubsystem<UChampionDataSubsystem>();
	if (Sub)
	{
		Sub->ApplyVisuals(this, ChampionData);
		Sub->ApplyStats(this, ChampionData);
	}

	CreateSkillExecutor();
}

// SkillExecutor 동적 생성 
void ALoLChampion::CreateSkillExecutor()
{
	if (!ChampionData || !ChampionData->SkillExecutorClass) { return; }

	if (SkillExecutor)
	{
		SkillExecutor->DestroyComponent();
	}

	SkillExecutor = NewObject<USkillExecutorComponent>(
		this, ChampionData->SkillExecutorClass, TEXT("SkillExecutor"));
	SkillExecutor->RegisterComponent();

	PRINTLOG_SH(TEXT("[%s] SkillExecutor 생성: %s"),
		*ChampionData->ChampionID.ToString(),
		*ChampionData->SkillExecutorClass->GetName());
}

// 스킬 활성화 → Executor 위임 
void ALoLChampion::HandleSkillActivated(ESkillSlot Slot, FVector TargetLoc)
{
	FVector direction = (TargetLoc - GetActorLocation()).GetSafeNormal2D();
	if (!direction.IsNearlyZero())
	{
		SetActorRotation(direction.Rotation());
	}
		
	if (!SkillExecutor) { return; }
	SkillExecutor->Execute(Slot, TargetLoc);
}

void ALoLChampion::StartAttackLoop(AActor* Target)
{
	if (!HasAuthority() || !Target) { return; }

	UTargetingComponent* TargetComp = Cast<ALoLChampion>(Target)->FindComponentByClass<UTargetingComponent>();
	if (!TargetComp || !TargetComp->IsValidTarget(this)) { return; }

	AttackTarget = Target;
	GetWorldTimerManager().ClearTimer(AttackLoopTimer);
	AttackLoopTick();
}

void ALoLChampion::StopAttackLoop()
{
	AttackTarget = nullptr;
	GetWorldTimerManager().ClearTimer(AttackLoopTimer);
	GetWorldTimerManager().ClearTimer(BasicAttackImpactTimer);

	if (StateComp && StateComp->GetCurrentState() == ECharacterState::BasicAttacking)
	{
		StateComp->TryChangeState(ECharacterState::Idle);
	}
}

void ALoLChampion::AttackLoopTick()
{
	if (!AttackTarget.IsValid())
	{
		StopAttackLoop();
		return;
	}

	AActor* Target = AttackTarget.Get();

	UTargetingComponent* TargetComp = Target->FindComponentByClass<UTargetingComponent>();
	if (!TargetComp || !TargetComp->IsValidTarget(this))
	{
		StopAttackLoop();
		return;
	}

	const float Range = StatComp ? StatComp->GetAttackRange() : 150.f;
	const float AttackSpeed = StatComp ? StatComp->GetAttackSpeed() : 0.65f;
	const float Dist = FVector::Dist2D(GetActorLocation(), Target->GetActorLocation());

	if (Dist > Range)
	{
		// 사거리 밖 → 이동
		StateComp->TryChangeState(ECharacterState::Moving);

		if (AController* Ctrl = GetController())
		{
			UAIBlueprintHelperLibrary::SimpleMoveToActor(Ctrl, Target);
		}

		GetWorldTimerManager().SetTimer(AttackLoopTimer, this, &ALoLChampion::AttackLoopTick, 0.1f, false);
		return;
	}

	// 사거리 안 → 공격
	if (AController* Ctrl = GetController())
	{
		Ctrl->StopMovement();
	}

	StateComp->TryChangeState(ECharacterState::BasicAttacking);

	FVector Dir = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	if (!Dir.IsNearlyZero())
	{
		SetActorRotation(Dir.Rotation());
	}

	ExecuteBasicAttack(Target);

	GetWorldTimerManager().SetTimer(AttackLoopTimer, this, &ALoLChampion::AttackLoopTick, 1.0f / AttackSpeed, false);
}

void ALoLChampion::OnDeath(AActor* DamageInstigator)
{
	StopAttackLoop();

	if (ChampionData && ChampionData->DeathMontage)
	{
		Multicast_PlayMontage(ChampionData->DeathMontage);
	}

	Super::OnDeath(DamageInstigator);

	// 서버에서만 리스폰 타이머 등록
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(RespawnTimer, this, &ALoLChampion::Respawn, RespawnDelay, false);
	}
}

void ALoLChampion::Respawn()
{
	if (!HasAuthority()) return;

	// HP 회복
	StatComp->ApplyHealthChange(StatComp->GetMaxHP());

	// Dead/Untargetable 태그 제거
	TagComp->RemoveTag(UnitTags::Dead);
	TagComp->RemoveTag(UnitTags::Untargetable);

	// 상태 Idle로 복귀
	StateComp->TryChangeState(ECharacterState::Idle);

	// 충돌 다시 활성화 (Multicast_OnDeath에서 껐으므로)
	Multicast_Respawn();
}

void ALoLChampion::Multicast_Respawn_Implementation()
{
	// 충돌 복구
	SetActorEnableCollision(true);

	// 리스폰 위치로 이동 (BeginPlay 시 기록해둔 스폰 위치)
	// 간단히 원래 위치 쓰거나, PlayerStart 위치 쓰면 됨
	// 지금은 제자리 리스폰
}

// 평타
void ALoLChampion::ExecuteBasicAttack(AActor* Target)
{
	if (!HasAuthority() || !Target) { return; }

	if (ChampionData && ChampionData->BasicAttackMontage)
		Multicast_PlayMontage(ChampionData->BasicAttackMontage);

	// 발사체로 평타 처리
	if (SkillExecutor && SkillExecutor->ProjectileClass)
	{
		FDamageContext Ctx;
		Ctx.RawDamage        = StatComp ? StatComp->GetAD() : 0.f;
		Ctx.DamageType       = EDamageType::Physical;
		Ctx.DamageInstigator = this;
		Ctx.SourceTag        = TEXT("BasicAttack");

		FVector Dir = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
		SkillExecutor->SpawnProjectile(Dir, 1800.f, 800.f, Ctx, false, false, TEXT("Socket_Q"));
		return;
	}

	// 발사체 없으면 타이머로 직접 데미지
	TWeakObjectPtr<AActor> WeakTarget(Target);
	GetWorldTimerManager().SetTimer(BasicAttackImpactTimer,
		[this, WeakTarget]()
		{
			if (WeakTarget.IsValid() && CombatComp)
			{
				CombatComp->PerformBasicAttack(WeakTarget.Get());
			}
		},
		0.3f, false);
}
