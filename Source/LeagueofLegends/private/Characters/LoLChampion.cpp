// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/LoLChampion.h"

#include "LeagueofLegends.h"
#include "Characters/Data/ChampionData.h"
#include "Champions/Projectile/ChampionSkillProjectile.h"
#include "Components/CombatComponent.h"
#include "Components/StatComponent.h"
#include "Components/SkillComponent.h"
#include "Components/SkillExecutorComponent.h"
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

	if (!ChampionData) return;

	UChampionDataSubsystem* Sub = GetGameInstance()->GetSubsystem<UChampionDataSubsystem>();
	if (!Sub) return;

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
		Sub->ApplyVisuals(this, ChampionData);
}

// ChampionData 세팅 (런타임, 캐릭터 선택 후) 
void ALoLChampion::SetChampionData(UChampionData* Data)
{
	if (!HasAuthority() || !Data) return;

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
	if (!ChampionData || !ChampionData->SkillExecutorClass) return;

	if (SkillExecutor)
		SkillExecutor->DestroyComponent();

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
	if (!SkillExecutor) return;
	SkillExecutor->Execute(Slot, TargetLoc);
}

// 평타 
void ALoLChampion::ExecuteBasicAttack(AActor* Target)
{
	if (!HasAuthority() || !Target) return;

	Multicast_PlayMontage(ChampionData ? ChampionData->BasicAttackMontage : nullptr);

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
				CombatComp->PerformBasicAttack(WeakTarget.Get());
		},
		0.3f, false);
}
