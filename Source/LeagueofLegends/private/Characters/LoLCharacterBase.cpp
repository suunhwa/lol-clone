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
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "UI/HPBarWidget.h"

ALoLCharacterBase::ALoLCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
	
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

	HPBarWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBarWidgetComp"));
	HPBarWidgetComp->SetupAttachment(GetRootComponent());
	HPBarWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	HPBarWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	HPBarWidgetComp->SetDrawSize(FVector2D(100.f, 12.f));
}

void ALoLCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		TagComp->SetTeam(InitialTeam);
		CombatComp->OnDeath.AddUObject(this, &ALoLCharacterBase::OnDeath);
	}

	if (UHPBarWidget* HPBar = Cast<UHPBarWidget>(HPBarWidgetComp->GetWidget()))
	{
		HPBar->InitWidget(StatComp);
	}
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
	if (!Montage) { return; }
	PlayAnimMontage(Montage);
}

void ALoLCharacterBase::Multicast_PlayMontageSection_Implementation(UAnimMontage* Montage, FName SectionName)
{
	if (!Montage) { return; }
	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	if (!Anim) { return; }
	Anim->Montage_Play(Montage);
	Anim->Montage_JumpToSection(SectionName, Montage);
}

void ALoLCharacterBase::Multicast_OnDeath_Implementation()
{
	SetActorEnableCollision(false);
}
