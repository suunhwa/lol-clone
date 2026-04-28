#include "Characters/Minion/Ranged_Projectile.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Characters/LoLMinion.h"
#include "Characters/Nexus/Nexus.h"

ALoLRanged_Projectile::ALoLRanged_Projectile()
{
    // 1. 충돌체 설정
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->InitSphereRadius(15.0f);
    
    // 콜리전 프리셋 설정 (미니언 캡슐을 Block할 수 있는 설정이어야 함)
    CollisionComp->SetCollisionProfileName(TEXT("Projectile")); 
    CollisionComp->OnComponentHit.AddDynamic(this, &ALoLRanged_Projectile::OnProjectileHit);
    RootComponent = CollisionComp;

    // 2. 외형 설정
    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(RootComponent);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 3. 이동 컴포넌트 설정
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->ProjectileGravityScale = 0.0f; // 중력 무시
    ProjectileMovement->InitialSpeed = 0.f;
    ProjectileMovement->MaxSpeed = 5000.f;

    SetLifeSpan(3.0f); // 3초 뒤 자동 파괴
}

void ALoLRanged_Projectile::Launch(float InSpeed, float InDamage, FName InTeamTag)
{
    Damage = InDamage;
    OwnerTeamTag = InTeamTag;

    if (ProjectileMovement)
    {
        // 소환된 시점의 정면 방향으로 발사
        ProjectileMovement->Velocity = GetActorForwardVector() * InSpeed;
    }
}

void ALoLRanged_Projectile::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // 나 자신이나 주인은 무시
    if (!OtherActor || OtherActor == GetOwner()) return;

    // 팀이 다른 경우에만 데미지
    if (!OtherActor->ActorHasTag(OwnerTeamTag))
    {
        if (ALoLMinion* TargetMinion = Cast<ALoLMinion>(OtherActor))
        {
            TargetMinion->TakeDamageSimple(Damage);
        }
        else if (class ANexus* TargetNexus = Cast<ANexus>(OtherActor))
        {
            TargetNexus->ReceiveDamage(Damage);
        }

        // 데미지를 줬다면 파괴
        Destroy();
    }
}