// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/LoLCharacterBase.h"

#include "LeagueofLegends.h"
#include "Net/UnrealNetwork.h"
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
#include "FOW/FOWManager.h"
#include "GameFramework/RiftGameState.h"
#include "GameFramework/RiftPlayerState.h"
#include "Components/WidgetComponent.h"
#include "UI/View/HPBarWidget.h"

ALoLCharacterBase::ALoLCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Champion"));

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

void ALoLCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALoLCharacterBase, FacingRotation);
}

void ALoLCharacterBase::FaceRotation(FRotator NewControlRotation, float DeltaTime)
{
	Super::FaceRotation(NewControlRotation, DeltaTime);

	if (HasAuthority())
	{
		FacingRotation = GetActorRotation();
	}
}

void ALoLCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (HasAuthority())
	{
		if (ARiftPlayerState* PS = GetPlayerState<ARiftPlayerState>())
		{
			TagComp->SetTeam(PS->GetTeam());
		}
	}
}

void ALoLCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		TagComp->SetTeam(InitialTeam);
		CombatComp->OnDeath.AddUObject(this, &ALoLCharacterBase::OnDeath);
	}

	/*auto* GS = GetWorld()->GetGameState<ARiftGameState>();                                                                                  
	GS->GetFOWManager()->RegisterSightProvider(this);    */
	

	if (auto* GS = GetWorld()->GetGameState<ARiftGameState>())
	{
		if (AFOWManager* FOWManager = GS->GetFOWManager())
		{
			FOWManager->RegisterSightProvider(this);
		}
		else
		{
			PRINTLOG_TK(TEXT("RegisterSightProvider failed: FOWManager is null for %s"), *GetName());
		}
	}
	else
	{
		PRINTLOG_TK(TEXT("RegisterSightProvider failed: GameState is null for %s"), *GetName());
	}

	if (UHPBarWidget* HPBar = Cast<UHPBarWidget>(HPBarWidgetComp->GetWidget()))
	{
		HPBar->InitWidget(StatComp);
	}
}

void ALoLCharacterBase::ReceiveDamage_Implementation(float Amount, EDamageType DamageType, AActor* DamageInstigator)
{
	if (!HasAuthority()) { return; }

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

	// Owner CombatComp ?�는 경우 (DoT, ?�경 ?��?지 ?? 직접 ?�용
	StatComp->ApplyHealthChange(-Amount);
}

bool ALoLCharacterBase::IsDead_Implementation() const
{
	return StatComp->IsDead();
}

bool ALoLCharacterBase::IsTargetable_Implementation() const
{
	return !IsDead() && !TagComp->HasTag(UnitTags::Untargetable);
}

FVector ALoLCharacterBase::GetTargetLocation_Implementation() const
{
	return GetActorLocation();
}

ETeam ALoLCharacterBase::GetTeam_Implementation() const
{
	return TagComp->GetTeam();
}

EUnitType ALoLCharacterBase::GetUnitType_Implementation() const
{
	return TagComp->GetUnitType();
}

AActor* ALoLCharacterBase::GetCurrentCombatTarget_Implementation() const
{
	return TargetingComp ? TargetingComp->GetCurrentTarget() : nullptr;
}

FVector ALoLCharacterBase::GetSightOrigin_Implementation() const
{
	return GetActorLocation();
}

float ALoLCharacterBase::GetSightRange_Implementation() const
{
	return SightRange;
}

bool ALoLCharacterBase::IsStatic_Implementation() const
{
	return bStaticSight;
}

ERiftSightTag ALoLCharacterBase::GetSightTag_Implementation() const
{
	return TagComp->GetSightTag();
}

bool ALoLCharacterBase::IsHideable_Implementation() const
{
	return false; 
}

void ALoLCharacterBase::OnDeath(AActor* DamageInstigator)
{
	Multicast_OnDeath();
}

void ALoLCharacterBase::Multicast_StartCooldown_Implementation(FName Tag, float Duration)
{
	// 서버는 StartCooldown에서 이미 처리했으므로 클라이언트만 적용
	if (!HasAuthority() && CooldownComp)
		CooldownComp->StartCooldown(Tag, Duration);
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
