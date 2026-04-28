// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/SkillExecutorComponent.h"

#include "Characters/ChampionSkillProjectile.h"
#include "Characters/LoLCharacterBase.h"
#include "Components/CooldownComponent.h"
#include "Components/StatComponent.h"

USkillExecutorComponent::USkillExecutorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USkillExecutorComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerChar  = Cast<ALoLCharacterBase>(GetOwner());
	if (!OwnerChar) return;

	StatComp     = OwnerChar->StatComp;
	CombatComp   = OwnerChar->CombatComp;
	CooldownComp = OwnerChar->CooldownComp;
}

void USkillExecutorComponent::PlayMontage(UAnimMontage* Montage) const
{
	if (OwnerChar)
		OwnerChar->Multicast_PlayMontage(Montage);
}

AChampionSkillProjectile* USkillExecutorComponent::SpawnProjectile(
	const FVector& Direction, float Speed, float Range,
	FDamageContext Ctx, bool bPiercing, bool bCooldownOnHit) const
{
	if (!ProjectileClass || !OwnerChar) return nullptr;

	const FVector SpawnLoc = OwnerChar->GetActorLocation() + FVector(0.f, 0.f, 50.f);

	AChampionSkillProjectile* Proj = OwnerChar->GetWorld()->SpawnActor<AChampionSkillProjectile>(
		ProjectileClass, SpawnLoc, Direction.Rotation());

	if (!Proj) return nullptr;

	Proj->SetOwner(OwnerChar);
	Proj->Launch(Ctx, Speed, Range, bPiercing, bCooldownOnHit);
	return Proj;
}
