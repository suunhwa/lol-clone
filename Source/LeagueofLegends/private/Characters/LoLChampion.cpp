// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/LoLChampion.h"

#include "LeagueofLegends.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Characters/Data/ChampionData.h"
#include "Champions/Projectile/ChampionSkillProjectile.h"
#include "Components/CombatComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/StatComponent.h"
#include "Components/StateComponent.h"
#include "Components/TagComponent.h"
#include "Components/SkillComponent.h"
#include "Components/SkillExecutorComponent.h"
#include "Components/StatModifierComponent.h"
#include "Components/TargetingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Interfaces/Damageable.h"
#include "Interfaces/Targetable.h"
#include "Manager/ChampionDataSubsystem.h"
#include "GameFramework/RiftHUD.h"
#include "GameFramework/RiftGameMode.h"
#include "GameFramework/RiftPlayerState.h"
#include "Net/UnrealNetwork.h"

ALoLChampion::ALoLChampion()
{
	PrimaryActorTick.bCanEverTick = true;
	
	StatModifierComp = CreateDefaultSubobject<UStatModifierComponent>(TEXT("StatModifierComp"));
	InventoryComp = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComp"));
}

void ALoLChampion::BeginPlay()
{
	Super::BeginPlay();

	if (SkillComp)
	{
		SkillComp->OnSkillActivated.AddUObject(this, &ALoLChampion::HandleSkillActivated);
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

	// 클라이언트 HUD 아이콘 갱신
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ARiftHUD* HUD = Cast<ARiftHUD>(PC->GetHUD()))
		{
			HUD->RefreshSkillIcons(this);
		}
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

	UTargetingComponent* TargetComp = Target->FindComponentByClass<UTargetingComponent>();
	if (TargetComp)
	{
		if (!TargetComp->IsValidTarget(this)) { return; }
	}
	else
	{
		// 포탑 등 TargetingComponent 없는 경우 인터페이스로 직접 검증
		if (!Target->GetClass()->ImplementsInterface(UDamageable::StaticClass())) { return; }
		if (IDamageable::Execute_IsDead(Target)) { return; }
		if (Target->GetClass()->ImplementsInterface(UTargetable::StaticClass()))
		{
			ETeam TargetTeam = ITargetable::Execute_GetTeam(Target);
			ETeam MyTeam = TagComp ? TagComp->GetTeam() : ETeam::None;
			if (TargetTeam == MyTeam || TargetTeam == ETeam::None) { return; }
		}
	}

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
	if (TargetComp)
	{
		if (!TargetComp->IsValidTarget(this))
		{
			StopAttackLoop();
			return;
		}
	}
	else
	{
		// 포탑 등 TargetingComponent 없는 경우 인터페이스로 직접 검증
		bool bValid = false;
		if (Target->GetClass()->ImplementsInterface(UDamageable::StaticClass()) &&
			!IDamageable::Execute_IsDead(Target) &&
			Target->GetClass()->ImplementsInterface(UTargetable::StaticClass()))
		{
			ETeam TargetTeam = ITargetable::Execute_GetTeam(Target);
			ETeam MyTeam = TagComp ? TagComp->GetTeam() : ETeam::None;
			bValid = (TargetTeam != MyTeam && TargetTeam != ETeam::None);
		}
		if (!bValid)
		{
			StopAttackLoop();
			return;
		}
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

	// 이동 즉시 차단 (죽은 채로 걷는 좀비 방지)
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
	}

	if (HasAuthority())
	{
		if (ARiftGameMode* GM = GetWorld()->GetAuthGameMode<ARiftGameMode>())
		{
			ALoLChampion* KillerChamp = Cast<ALoLChampion>(DamageInstigator);
			ARiftPlayerState* KillerPS = KillerChamp ? KillerChamp->GetPlayerState<ARiftPlayerState>() : nullptr;
			ARiftPlayerState* VictimPS = GetPlayerState<ARiftPlayerState>();
			// GM->OnChampionKilled(KillerPS, VictimPS);
			
			// 사망 직전 10초 이내에 공격한 적 플레이어 수집
			TArray<ARiftPlayerState*> Assisters;
			if (CombatComp)
			{
				Assisters = CombatComp->GetAssisters(KillerPS);
				CombatComp->ClearAssisters();
			}

			GM->OnChampionKilled(KillerPS, VictimPS, Assisters);
		}
	}

	if (ChampionData && ChampionData->DeathMontage)
	{
		Multicast_PlayMontage(ChampionData->DeathMontage);
	}

	Super::OnDeath(DamageInstigator);

	// 리스폰 비활성화 (테스트용)
	// if (HasAuthority())
	// {
	// 	GetWorldTimerManager().SetTimer(RespawnTimer, this, &ALoLChampion::Respawn, RespawnDelay, false);
	// }
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
	SetActorEnableCollision(true);

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_Walking);
	}

	if (HPBarWidgetComp)
	{
		HPBarWidgetComp->SetVisibility(true);
	}
}

// 평타
void ALoLChampion::ExecuteBasicAttack(AActor* Target)
{
	if (!HasAuthority() || !Target) { return; }

	if (ChampionData && ChampionData->BasicAttackMontage)
	{
		Multicast_PlayMontage(ChampionData->BasicAttackMontage);
	}

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
