#include "Characters/Object/LoLTower.h"
#include "Components/ObjectStatComponent.h"
#include "Kismet/GameplayStatics.h"

ALoLTower::ALoLTower()
{
    ObjectID = 11001;
}

void ALoLTower::BeginPlay()
{
    Super::BeginPlay();
    
    float Interval = (ObjectStatComp->GetAttackSpeed() > 0.f) ? (1.f / ObjectStatComp->GetAttackSpeed()) : 1.2f;
    GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &ALoLTower::CheckAndAttack, Interval, true);
}

void ALoLTower::CheckAndAttack()
{
    if (IsDead()) return;
    
    
    if (IsValid(CurrentTarget)) Fire();
    else ObjectStatComp->ResetHeatingStack();
}

void ALoLTower::Fire()
{
    IDamageable* Target = Cast<IDamageable>(CurrentTarget);
    if (Target)
    {
        Target->ReceiveDamage(ObjectStatComp->GetAttackDamage(), EDamageType::Physical, this);
        ObjectStatComp->AddHeatingStack();
    }
}