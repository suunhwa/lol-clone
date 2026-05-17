// Fill out your copyright notice in the Description page of Project Settings.

#include "Champions/Projectile/ChampionSkillProjectile.h"

#include "DrawDebugHelpers.h"
#include "Characters/LoLCharacterBase.h"
#include "Components/CooldownComponent.h"
#include "Components/StatComponent.h"
#include "Interfaces/Damageable.h"
#include "Interfaces/Targetable.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Type/RiftTypes.h"

AChampionSkillProjectile::AChampionSkillProjectile()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	CollisionComp->InitSphereRadius(60.f);
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

void AChampionSkillProjectile::SetCollisionRadius(float Radius)
{
	if (CollisionComp)
		CollisionComp->SetSphereRadius(Radius);
}

void AChampionSkillProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const FVector Loc = GetActorLocation();

	// R은 인디케이터가 이미 경로를 보여주므로 trail 생략
	if (!PrevLocation.IsZero() && !PrevLocation.Equals(Loc, 1.f) && DebugTrailHalfWidth <= 0.f)
	{
		// Q/W/E: 얇은 선 trail
		// DrawDebugLine(GetWorld(), PrevLocation, Loc, FColor::Cyan, false, 0.5f, 0, 6.f);
	}

	PrevLocation = Loc;
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
	if (!Other->GetClass()->ImplementsInterface(UTargetable::StaticClass())) { return; }

	ALoLCharacterBase* Caster = Cast<ALoLCharacterBase>(GetOwner());
	if (!Caster) { return; }
	if (ITargetable::Execute_GetTeam(Other) == ITargetable::Execute_GetTeam(Caster)) { return; }

	ApplyHit(Other);
	Destroy();
}

void AChampionSkillProjectile::OnBeginOverlap(UPrimitiveComponent* Overlapped, AActor* Other,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !Other || Other == GetOwner()) { return; }
	if (HitActors.Contains(Other)) { return; }
	if (!Other->GetClass()->ImplementsInterface(UTargetable::StaticClass())) { return; }

	ALoLCharacterBase* Caster = Cast<ALoLCharacterBase>(GetOwner());
	if (!Caster) { return; }
	if (ITargetable::Execute_GetTeam(Other) == ITargetable::Execute_GetTeam(Caster)) { return; }

	HitActors.Add(Other);
	ApplyHit(Other);

	// 관통 아닌 발사체(Q/E)는 첫 타겟 적중 후 소멸
	if (!bIsPiercing)
	{
		Destroy();
	}
}

void AChampionSkillProjectile::ApplyHit(AActor* Target)
{
	ALoLCharacterBase* Caster = Cast<ALoLCharacterBase>(GetOwner());
	if (!Caster || !Caster->CombatComp) { return; }

	// 구조물(타워/억제기/넥서스) 피해 차단 플래그
	if (!bCanDamageStructures && Target->GetClass()->ImplementsInterface(UTargetable::StaticClass()))
	{
		EUnitType Type = ITargetable::Execute_GetUnitType(Target);
		if (Type == EUnitType::Tower || Type == EUnitType::Inhibitor || Type == EUnitType::Nexus)
		{
			OnHitDelegate.ExecuteIfBound(Target);
			return;
		}
	}

	// StatComponent 있는 타겟(챔피언, 미니언)은 CombatComp 경유
	// 없는 타겟(포탑 등)은 IDamageable 인터페이스로 직접 전달
	if (Target->FindComponentByClass<UStatComponent>())
	{
		Caster->CombatComp->DealDamage(Target, DamageCtx);
	}
	else if (Target->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
	{
		IDamageable::Execute_ReceiveDamage(Target, DamageCtx.RawDamage, DamageCtx.DamageType, Caster);
	}

	OnHitDelegate.ExecuteIfBound(Target);

	if (bCooldownOnHit && Caster->CooldownComp)
	{
		Caster->CooldownComp->ReduceAllCooldowns(1.5f);
	}
}
