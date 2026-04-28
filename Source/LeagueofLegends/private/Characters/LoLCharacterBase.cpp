// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/LoLCharacterBase.h"
#include "Components/StatComponent.h"
#include "Components/CombatComponent.h"
#include "Components/TagComponent.h"
#include "Components/StateComponent.h"
#include "Components/StatusEffectComponent.h"
#include "Components/CooldownComponent.h"
#include "Components/SkillComponent.h"
#include "Components/TargetingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ALoLCharacterBase::ALoLCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);

	StatComp = CreateDefaultSubobject<UStatComponent>(TEXT("StatComp"));
	CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComp"));
	TagComp = CreateDefaultSubobject<UTagComponent>(TEXT("TagComp"));
	StateComp = CreateDefaultSubobject<UStateComponent>(TEXT("StateComp"));
	StatusEffectComp = CreateDefaultSubobject<UStatusEffectComponent>(TEXT("StatusEffectComp"));
	CooldownComp = CreateDefaultSubobject<UCooldownComponent>(TEXT("CooldownComp"));
	SkillComp = CreateDefaultSubobject<USkillComponent>(TEXT("SkillComp"));
	TargetingComp = CreateDefaultSubobject<UTargetingComponent>(TEXT("TargetingComp"));
}

void ALoLCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
		CombatComp->OnDeath.AddUObject(this, &ALoLCharacterBase::OnDeath);
}

void ALoLCharacterBase::ReceiveDamage(float Amount, EDamageType DamageType, AActor* DamageInstigator)
{
	if (!HasAuthority()) return;

	FDamageContext Ctx;
	Ctx.RawDamage = Amount;
	Ctx.DamageType = DamageType;
	Ctx.DamageInstigator = DamageInstigator;

	if (DamageInstigator)
	{
		UCombatComponent* InstigatorCombat = DamageInstigator->FindComponentByClass<UCombatComponent>();
		if (InstigatorCombat)
		{
			InstigatorCombat->DealDamage(this, Ctx);
			return;
		}
	}

	// Owner CombatComp 없는 경우 (DoT, 환경 데미지 등) 직접 적용
	StatComp->ApplyHealthChange(-Amount);
}

bool ALoLCharacterBase::IsDead() const
{
	return StatComp->IsDead();
}

bool ALoLCharacterBase::IsTargetable() const
{
	return !IsDead() && !TagComp->HasTag(UnitTags::Untargetable);
}

FVector ALoLCharacterBase::GetTargetLocation() const
{
	return GetActorLocation();
}

ETeam ALoLCharacterBase::GetTeam() const
{
	return TagComp->GetTeam();
}

void ALoLCharacterBase::OnDeath(AActor* DamageInstigator)
{
	Multicast_OnDeath();
}

void ALoLCharacterBase::Multicast_PlayMontage_Implementation(UAnimMontage* Montage)
{
	if (!Montage) return;
	PlayAnimMontage(Montage);
}

void ALoLCharacterBase::Multicast_OnDeath_Implementation()
{
	SetActorEnableCollision(false);
}
