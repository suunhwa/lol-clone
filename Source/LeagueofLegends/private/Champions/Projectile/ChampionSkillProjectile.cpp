// Fill out your copyright notice in the Description page of Project Settings.

#include "Champions/Projectile/ChampionSkillProjectile.h"

#include "Characters/LoLCharacterBase.h"
#include "Components/CooldownComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AChampionSkillProjectile::AChampionSkillProjectile()
{
	bReplicates = true;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	CollisionComp->InitSphereRadius(20.f);
	CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
	CollisionComp->OnComponentHit.AddDynamic(this, &AChampionSkillProjectile::OnHit);
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AChampionSkillProjectile::OnBeginOverlap);
	RootComponent = CollisionComp;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->InitialSpeed = 0.f;
	ProjectileMovement->MaxSpeed = 5000.f;
}

void AChampionSkillProjectile::Launch(FDamageContext InCtx, float Speed, float MaxRange, bool bPiercing, bool bInCooldownOnHit)
{
	DamageCtx = InCtx;
	bIsPiercing = bPiercing;
	bCooldownOnHit = bInCooldownOnHit;

	if (bPiercing)
	{
		// 관통: Pawn에 Overlap (통과하며 피해)
		CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		CollisionComp->SetGenerateOverlapEvents(true);
	}

	ProjectileMovement->Velocity = GetActorForwardVector() * Speed;
	SetLifeSpan(MaxRange / Speed);
}

void AChampionSkillProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* Other,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority() || !Other || Other == GetOwner()) { return; }

	ALoLCharacterBase* OtherChar = Cast<ALoLCharacterBase>(Other);
	ALoLCharacterBase* Caster = Cast<ALoLCharacterBase>(GetOwner());
	if (!OtherChar || !Caster || ITargetable::Execute_GetTeam(OtherChar) == ITargetable::Execute_GetTeam(Caster)) { return; }

	ApplyHit(Other);
	Destroy();
}

void AChampionSkillProjectile::OnBeginOverlap(UPrimitiveComponent* Overlapped, AActor* Other,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !Other || Other == GetOwner()) { return; }
	if (HitActors.Contains(Other)) { return; }

	ALoLCharacterBase* OtherChar = Cast<ALoLCharacterBase>(Other);
	ALoLCharacterBase* Caster = Cast<ALoLCharacterBase>(GetOwner());
	if (!OtherChar || !Caster || ITargetable::Execute_GetTeam(OtherChar) == ITargetable::Execute_GetTeam(Caster)) { return; }

	HitActors.Add(Other);
	ApplyHit(Other);
}

void AChampionSkillProjectile::ApplyHit(AActor* Target)
{
	ALoLCharacterBase* Caster = Cast<ALoLCharacterBase>(GetOwner());
	if (!Caster || !Caster->CombatComp) { return; }

	Caster->CombatComp->DealDamage(Target, DamageCtx);

	if (bCooldownOnHit && Caster->CooldownComp)
	{
		Caster->CooldownComp->ReduceAllCooldowns(1.5f);
	}
}
