// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/LoLChampion.h"

#include "LeagueofLegends.h"
#include "Characters/Data/ChampionData.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
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
#include "Components/DecalComponent.h"
#include "Interfaces/Damageable.h"
#include "Interfaces/Targetable.h"
#include "Manager/ChampionDataSubsystem.h"
#include "GameFramework/RiftHUD.h"
#include "Characters/LoLChampionRespawnPoint.h"
#include "GameFramework/LoLCameraActor.h"
#include "GameFramework/RiftGameMode.h"
#include "GameFramework/RiftPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ALoLChampion::ALoLChampion()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;  // 직접 SetActorRotation으로 제어
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);

	StatModifierComp = CreateDefaultSubobject<UStatModifierComponent>(TEXT("StatModifierComp"));
	InventoryComp = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComp"));

	RecallGroundComp = CreateDefaultSubobject<UDecalComponent>(TEXT("RecallGroundComp"));
	RecallGroundComp->SetupAttachment(GetRootComponent());
	RecallGroundComp->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	RecallGroundComp->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));  // 아래 방향 투영
	RecallGroundComp->DecalSize = FVector(1.f, 50.f, 50.f);
	RecallGroundComp->SetVisibility(false);

	// StatComp는 LoLCharacterBase 생성자에서 먼저 생성됨 — 여기서 주입
	if (StatComp)
	{
		StatComp->SetStatModifierComp(StatModifierComp);
	}
}

void ALoLChampion::PostNetReceiveLocationAndRotation()
{
	// AutonomousProxy(로컬 클라이언트 자신)에도 서버 복제 위치를 부드럽게 적용.
	// 기본 구현은 SimulatedProxy에서만 적용하므로, 서버 권위 이동 시스템에서는 오버라이드 필요.
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		const FRepMovement& RepMove = GetReplicatedMovement();
		const FVector OldLoc = GetActorLocation();
		const FQuat OldQuat = GetActorQuat();

		SetActorLocationAndRotation(RepMove.Location, RepMove.Rotation, false, nullptr, ETeleportType::None);

		if (GetCharacterMovement())
		{
			GetCharacterMovement()->SmoothCorrection(OldLoc, OldQuat, RepMove.Location, RepMove.Rotation.Quaternion());
		}
		return;
	}
	Super::PostNetReceiveLocationAndRotation();
}

void ALoLChampion::BeginPlay()
{
	Super::BeginPlay();

	if (SkillComp)
	{
		SkillComp->OnSkillActivated.AddUObject(this, &ALoLChampion::HandleSkillActivated);
	}

	// 이동/공격 시작 시 귀환 도착 애니 중단 (모든 클라이언트에서 동작)
	if (StateComp)
	{
		StateComp->OnStateChanged.AddWeakLambda(this, [this](ECharacterState OldState, ECharacterState NewState)
		{
			if (NewState == ECharacterState::Moving
				|| NewState == ECharacterState::BasicAttacking
				|| NewState == ECharacterState::CastingSkill)
			{
				UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
				if (Anim && ChampionData
					&& (Anim->Montage_IsPlaying(ChampionData->RecallLandMontage)
						|| Anim->Montage_IsPlaying(ChampionData->RecallExitMontage)))
				{
					Anim->StopAllMontages(0.2f);
				}
			}
		});
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
	DOREPLIFETIME(ALoLChampion, AnimVelocity);
}

void ALoLChampion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateMovement(DeltaTime);
}

void ALoLChampion::OnRep_ChampionData()
{
	UChampionDataSubsystem* Sub = GetGameInstance()->GetSubsystem<UChampionDataSubsystem>();
	if (Sub && ChampionData)
	{
		Sub->ApplyVisuals(this, ChampionData);

		// 클라이언트에서도 MaxWalkSpeed 동기화 (ApplyStats는 서버 전용이라 직접 설정)
		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			if (StatComp && StatComp->GetMoveSpeed() > 0.f)
			{
				MoveComp->MaxWalkSpeed = StatComp->GetMoveSpeed();
			}
		}
	}

	// 컨트롤러가 아직 없으면 HUD 갱신은 PawnClientRestart에서 처리
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ARiftHUD* HUD = Cast<ARiftHUD>(PC->GetHUD()))
		{
			HUD->RefreshSkillIcons(this);
		}
	}
}

void ALoLChampion::PawnClientRestart()
{
	Super::PawnClientRestart();

	if (ChampionData)
	{
		UChampionDataSubsystem* Sub = GetGameInstance()->GetSubsystem<UChampionDataSubsystem>();
		if (Sub)
		{
			Sub->ApplyVisuals(this, ChampionData);
		}

		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (ARiftHUD* HUD = Cast<ARiftHUD>(PC->GetHUD()))
			{
				HUD->RefreshSkillIcons(this);
			}
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

// ----------------------- 이동 시스템 -----------------------

void ALoLChampion::SetMoveTargetFrom(FVector StartLoc, FVector Destination)
{
	UNavigationPath* Path = UNavigationSystemV1::FindPathToLocationSynchronously(
		GetWorld(), StartLoc, Destination);

	if (Path && Path->PathPoints.Num() >= 2)
	{
		PathPoints = Path->PathPoints;
	}
	else
	{
		PathPoints = {StartLoc, Destination};
	}

	PathIndex = 1;
	MoveDestination = Destination;
	bHasMoveTarget = true;

	if (HasAuthority() && StateComp)
	{
		StateComp->TryChangeState(ECharacterState::Moving);
	}
}

void ALoLChampion::SetMoveTarget(FVector Destination)
{
	SetMoveTargetFrom(GetActorLocation(), Destination);
}

void ALoLChampion::CancelMove()
{
	bHasMoveTarget = false;
	PathPoints.Empty();
	PathIndex = 0;
	AnimVelocity = FVector::ZeroVector;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
	}
}

void ALoLChampion::UpdateMovement(float DeltaTime)
{
	if (!HasAuthority()) { return; }
	if (!bHasMoveTarget || PathPoints.Num() == 0) { return; }

	const FVector CurrentLoc = GetActorLocation();

	// 현재 웨이포인트에 충분히 가까우면 다음으로 진행
	while (PathIndex < PathPoints.Num() &&
		FVector::Dist2D(CurrentLoc, PathPoints[PathIndex]) <= MoveAcceptanceRadius)
	{
		++PathIndex;
	}

	if (PathIndex >= PathPoints.Num())
	{
		// 목적지 도착
		bHasMoveTarget = false;
		PathPoints.Empty();
		AnimVelocity = FVector::ZeroVector;
		if (HasAuthority() && StateComp &&
			StateComp->GetCurrentState() == ECharacterState::Moving)
		{
			StateComp->TryChangeState(ECharacterState::Idle);
		}
		return;
	}

	const FVector Dir = (PathPoints[PathIndex] - CurrentLoc).GetSafeNormal2D();
	if (!Dir.IsNearlyZero())
	{
		const FVector MoveVelocity = Dir * GetCharacterMovement()->MaxWalkSpeed;
		AnimVelocity = MoveVelocity;
		GetCharacterMovement()->RequestDirectMove(MoveVelocity, false);
	}
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
		this,
		ChampionData->SkillExecutorClass,
		TEXT("SkillExecutor"));
	SkillExecutor->RegisterComponent();
}

// 스킬 활성화 → Executor 위임 
void ALoLChampion::HandleSkillActivated(ESkillSlot Slot, FVector TargetLoc)
{
	// 회전은 Server_RequestSkill에서 이미 처리 — 여기서 중복 호출 제거
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
	CancelMove();

	if (StateComp)
	{
		const ECharacterState Cur = StateComp->GetCurrentState();
		if (Cur == ECharacterState::BasicAttacking || Cur == ECharacterState::Moving)
		{
			StateComp->TryChangeState(ECharacterState::Idle);
		}
	}
}

void ALoLChampion::AttackLoopTick()
{
	if (!HasAuthority()) { return; }
	
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
	const float AttackSpeed = FMath::Max(StatComp ? StatComp->GetAttackSpeed() : 0.65f, 0.1f);
	const float Dist = FVector::Dist2D(GetActorLocation(), Target->GetActorLocation());

	if (Dist > Range)
	{
		// 사거리 밖 → 타겟 추적
		StateComp->TryChangeState(ECharacterState::Moving);
		SetMoveTarget(Target->GetActorLocation());

		GetWorldTimerManager().SetTimer(AttackLoopTimer, this, &ALoLChampion::AttackLoopTick, 0.1f, false);
		return;
	}

	// 사거리 안 → 이동 중단 후 공격
	CancelMove();

	StateComp->TryChangeState(ECharacterState::BasicAttacking);

	FVector Dir = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	if (HasAuthority() && !Dir.IsNearlyZero())
	{
		SetActorRotation(Dir.Rotation());
	}

	ExecuteBasicAttack(Target);

	GetWorldTimerManager().SetTimer(AttackLoopTimer,
	                                this,
	                                &ALoLChampion::AttackLoopTick,
	                                FMath::Max(1.0f / AttackSpeed, 0.1f),
	                                false);
}

void ALoLChampion::OnDeath(AActor* DamageInstigator)
{
	StopAttackLoop();
	CancelMove();

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
	// 사망 상태는 GameMode::OnChampionKilled에서 PlayerState에 복제됨
}

void ALoLChampion::StartRespawnTimer(float Delay)
{
	if (!HasAuthority()) { return; }
	PRINTLOG_SH(TEXT("[Respawn] 부활 타이머 %.0f초"), Delay);

	TWeakObjectPtr<ALoLChampion> WeakThis(this);
	GetWorldTimerManager().SetTimer(RespawnTimer,
		[WeakThis]()
		{
			if (!WeakThis.IsValid()) { return; }
			if (ARiftGameMode* GM = WeakThis->GetWorld()->GetAuthGameMode<ARiftGameMode>())
			{
				GM->RespawnChampion(WeakThis.Get());
			}
			else
			{
				WeakThis->Respawn(); // GameMode 없을 때 폴백
			}
		},
		Delay, false);
}

void ALoLChampion::Respawn()
{
	if (!HasAuthority()) { return; }

	// HP/마나 전체 회복
	if (StatComp)
	{
		StatComp->ApplyHealthChange(StatComp->GetMaxHP());
	}

	// Dead/Untargetable 태그 제거
	if (TagComp)
	{
		TagComp->RemoveTag(UnitTags::Dead);
		TagComp->RemoveTag(UnitTags::Untargetable);
	}

	// 상태 Idle로 복귀
	if (StateComp)
	{
		StateComp->TryChangeState(ECharacterState::Idle);
	}

	// 충돌·이동 복구 (모든 클라이언트)
	Multicast_Respawn();

	// 사망 상태 해제 (OnRep_DeathState가 카메라 효과 + UI 모두 클리어)
	if (ARiftPlayerState* PS = GetPlayerState<ARiftPlayerState>())
	{
		PS->SetDeadState(false, 0.f);
	}
	
	CancelMove();
}

void ALoLChampion::Multicast_Respawn_Implementation()
{
	SetActorEnableCollision(true);

	// 사망 몽타주 강제 종료
	if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
	{
		Anim->StopAllMontages(0.1f);
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_Walking);
	}

	if (HPBarWidgetComp)
	{
		HPBarWidgetComp->SetVisibility(true);
	}

	if (ChampionData && ChampionData->RespawnMontage)
	{
		PlayAnimMontage(ChampionData->RespawnMontage);
	}
}

// ----------------------- 귀환 시스템 -----------------------

void ALoLChampion::Server_RequestRecall_Implementation()
{
	StartRecall();
}

void ALoLChampion::StartRecall()
{
	if (!HasAuthority()) { return; }
	if (!StateComp || !StateComp->TryChangeState(ECharacterState::Recalling)) { return; }

	StopAttackLoop();
	CancelMove();

	bIsRecalling = true;

	if (ChampionData && ChampionData->RecallMontage)
	{
		Multicast_PlayMontage(ChampionData->RecallMontage);
	}

	Multicast_ShowRecallDecal();

	TWeakObjectPtr<ALoLChampion> WeakThis(this);
	GetWorldTimerManager().SetTimer(RecallTimer,
		[WeakThis]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->CompleteRecall();
			}
		},
		8.f, false);
}

void ALoLChampion::CancelRecall()
{
	if (!HasAuthority() || !bIsRecalling) { return; }

	bIsRecalling = false;
	GetWorldTimerManager().ClearTimer(RecallTimer);

	if (StateComp)
	{
		StateComp->TryChangeState(ECharacterState::Idle);
	}

	Multicast_CancelRecall();
}

void ALoLChampion::Multicast_CancelRecall_Implementation()
{
	if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
	{
		Anim->StopAllMontages(0.2f);
	}

	if (RecallGroundComp)
	{
		RecallGroundComp->SetVisibility(false);
	}
}

void ALoLChampion::Multicast_ShowRecallDecal_Implementation()
{
	if (RecallGroundComp)
	{
		RecallGroundComp->SetVisibility(true);
	}
}

void ALoLChampion::CompleteRecall()
{
	if (!HasAuthority()) { return; }

	bIsRecalling = false;

	// 팀 기지로 텔레포트
	if (ARiftGameMode* GM = GetWorld()->GetAuthGameMode<ARiftGameMode>())
	{
		const ARiftPlayerState* PS = GetPlayerState<ARiftPlayerState>();
		const ETeam MyTeam = PS ? PS->GetTeam() : ETeam::None;
		ALoLChampionRespawnPoint* Point = GM->FindRespawnPoint(MyTeam);
		if (Point)
		{
			const int32 SlotIndex = PS ? PS->GetTeamSlotIndex() : 0;
			const int32 Idx = SlotIndex % ALoLChampionRespawnPoint::SlotOffsets.Num();
			const FVector SpawnLoc = Point->GetActorLocation() + ALoLChampionRespawnPoint::SlotOffsets[Idx];
			SetActorLocationAndRotation(SpawnLoc, Point->GetActorRotation(), false, nullptr, ETeleportType::TeleportPhysics);
		}
	}

	// HP 전체 회복 (기지 우물 효과)
	if (StatComp)
	{
		StatComp->ApplyHealthChange(StatComp->GetMaxHP());
	}

	if (StateComp)
	{
		StateComp->TryChangeState(ECharacterState::Idle);
	}

	Multicast_PlayRecallLanding();
}

void ALoLChampion::Multicast_PlayRecallLanding_Implementation()
{
	if (RecallGroundComp)
	{
		RecallGroundComp->SetVisibility(false);
	}

	if (!ChampionData || !ChampionData->RecallLandMontage) { return; }

	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	if (!Anim) { return; }

	PlayAnimMontage(ChampionData->RecallLandMontage);

	// 착지 끝나면 Exit 자동 재생
	if (ChampionData->RecallExitMontage)
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindWeakLambda(this, [this](UAnimMontage* Montage, bool bInterrupted)
		{
			if (!bInterrupted && ChampionData && ChampionData->RecallExitMontage)
			{
				PlayAnimMontage(ChampionData->RecallExitMontage);
			}
		});
		Anim->Montage_SetEndDelegate(EndDelegate, ChampionData->RecallLandMontage);
	}
}

void ALoLChampion::OnDamageReceived()
{
	if (bIsRecalling)
	{
		CancelRecall();
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
		Ctx.RawDamage = StatComp ? StatComp->GetAD() : 0.f;
		Ctx.DamageType = EDamageType::Physical;
		Ctx.DamageInstigator = this;
		Ctx.SourceTag = TEXT("BasicAttack");

		FVector Dir = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
		AChampionSkillProjectile* Proj = SkillExecutor->SpawnProjectile(
			Dir,
			1800.f,
			800.f,
			Ctx,
			false,
			false,
			TEXT("Socket_Q"));

		// 챔피언별 평타 후처리 (ex. 이즈리얼 W 고리 소비)
		SkillExecutor->OnBasicAttackFired(Proj, Target);
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
	                                0.3f,
	                                false);
}
