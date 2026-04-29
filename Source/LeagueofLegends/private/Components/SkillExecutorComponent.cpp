// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/SkillExecutorComponent.h"

#include "LeagueofLegends.h"
#include "Champions/Projectile/ChampionSkillProjectile.h"
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
	FDamageContext Ctx, bool bPiercing, bool bCooldownOnHit, FName SocketName) const
{
	if (!ProjectileClass || !OwnerChar) return nullptr;

	FVector SpawnLoc = OwnerChar->GetActorLocation() + FVector(0.f, 0.f, 50.f);
	if (SocketName != NAME_None)
	{
		USkeletalMeshComponent* Mesh = OwnerChar->GetMesh();
		if (Mesh && Mesh->DoesSocketExist(SocketName))
			SpawnLoc = Mesh->GetSocketLocation(SocketName);
	}

	// 챔피언 캡슐 밖으로 살짝 앞으로 밀어서 스폰 (캡슐 충돌 방지)
	SpawnLoc += Direction * 60.f;

	AChampionSkillProjectile* Proj = OwnerChar->GetWorld()->SpawnActor<AChampionSkillProjectile>(
		ProjectileClass, SpawnLoc, Direction.Rotation());

	if (!Proj) return nullptr;

	Proj->SetOwner(OwnerChar);
	if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(Proj->GetRootComponent()))
	{
		Root->IgnoreActorWhenMoving(OwnerChar, true);
	}
	Proj->Launch(Ctx, Speed, Range, bPiercing, bCooldownOnHit);
	return Proj;
}
