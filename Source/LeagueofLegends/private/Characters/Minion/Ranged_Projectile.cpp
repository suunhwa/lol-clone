#include "Characters/Minion/Ranged_Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StatComponent.h"
#include "Components/TagComponent.h"
#include "Characters/LoLMinion.h"
#include "Characters/Nexus/Nexus.h"

ALoLRanged_Projectile::ALoLRanged_Projectile()
{
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->InitSphereRadius(15.0f);
    CollisionComp->SetCollisionProfileName(TEXT("Projectile")); 
    RootComponent = CollisionComp;

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(RootComponent);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->ProjectileGravityScale = 0.0f;
    ProjectileMovement->MaxSpeed = 5000.f;

    SetLifeSpan(3.0f);
}

void ALoLRanged_Projectile::Launch(AActor* InTarget, float InSpeed, float InDamage, ETeam InOwnerTeam)
{
    TargetActor = InTarget;
    Damage = InDamage;
    OwnerTeam = InOwnerTeam;

    if (ProjectileMovement)
    {
        ProjectileMovement->InitialSpeed = InSpeed;
        if (InTarget)
        {
            ProjectileMovement->bIsHomingProjectile = true;
            ProjectileMovement->HomingTargetComponent = InTarget->GetRootComponent();
            ProjectileMovement->HomingAccelerationMagnitude = InSpeed * 2.5f;
        }
        FVector Direction = InTarget ? (InTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal() : GetActorForwardVector();
        ProjectileMovement->Velocity = Direction * InSpeed;
    }
}

void ALoLRanged_Projectile::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);

    if (!HasAuthority() || !OtherActor || OtherActor == GetOwner()) return;

    // 타겟팅 투사체인 경우 타겟 필터링 (필요에 따라 제거 가능)
    if (TargetActor.IsValid() && OtherActor != TargetActor.Get()) return;

    // 상대방의 태그 컴포넌트 확인
    UTagComponent* OtherTag = OtherActor->FindComponentByClass<UTagComponent>();
    if (OtherTag)
    {
        // 적인지 확인 (내 팀과 상대 팀이 다를 때만 데미지)
        if (OwnerTeam != ETeam::None && OtherTag->GetTeam() != ETeam::None && OwnerTeam != OtherTag->GetTeam())
        {
            if (UStatComponent* TargetStat = OtherActor->FindComponentByClass<UStatComponent>())
            {
                TargetStat->ApplyHealthChange(-Damage);
            }
            else if (ANexus* TargetNexus = Cast<ANexus>(OtherActor))
            {
                TargetNexus->ReceiveDamage(Damage);
            }
            
            Destroy();
        }
    }
}