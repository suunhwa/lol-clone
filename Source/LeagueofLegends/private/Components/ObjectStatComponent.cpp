#include "Components/ObjectStatComponent.h"

#include "LeagueofLegends.h"
#include "Interfaces/Damageable.h"
#include "Interfaces/Targetable.h"
#include "Net/UnrealNetwork.h"

UObjectStatComponent::UObjectStatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UObjectStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UObjectStatComponent, CurrentHP);
    DOREPLIFETIME(UObjectStatComponent, CurrentArmor);
    DOREPLIFETIME(UObjectStatComponent, CurrentHeatStack);
}

void UObjectStatComponent::InitObjectStats(const FObjectBaseRow& Base, const FObjectRewardRow& Reward, const FObjectMechanicsRow& Mech)
{
    BaseData = Base;
    RewardData = Reward;
    MechData = Mech;

    CurrentHP = BaseData.Base_HP;
    CurrentArmor = MechData.Base_Armor_After_Expiry;
    
    // --- 데이터 로드 확인 로그 ---
    // GetOwner()->GetName()을 쓰면 어떤 포탑인지 이름까지 나옴!
    PRINTLOG_HJ(TEXT("------------------------------------------"));
    PRINTLOG_HJ(TEXT("[Stat Init] Owner: %s"), *GetOwner()->GetName());
    PRINTLOG_HJ(TEXT("ID: %d | Name: %s"), BaseData.Object_ID, *BaseData.Object_Type);
    PRINTLOG_HJ(TEXT("HP: %.1f | AD: %.1f | Range: %.1f"), BaseData.Base_HP, BaseData.Base_AD, BaseData.Atk_Range);
    PRINTLOG_HJ(TEXT("Armor: %.1f | HeatRate: %.2f"), MechData.Base_Armor_After_Expiry, MechData.Heating_Rate);
    PRINTLOG_HJ(TEXT("------------------------------------------"));
    
    OnHPChanged.Broadcast(CurrentHP, GetMaxHP());
}

float UObjectStatComponent::GetAttackDamage(AActor* Target) const
{
    float FinalDamage = BaseData.Base_AD;

    if (!IsValid(Target)) return FinalDamage;

    // 1. 타겟의 스탯 컴포넌트와 인터페이스 확인
    // 형진님의 프로젝트 구조상 미니언도 StatComponent(또는 MinionStatComponent)를 가지고 있을 것입니다.
    UObjectStatComponent* TargetStat = Target->FindComponentByClass<UObjectStatComponent>();
    ITargetable* TargetIT = Cast<ITargetable>(Target);
    
    if (TargetIT && TargetStat)
    {
        EUnitType TargetType = TargetIT->Execute_GetUnitType(Target);

        // 1. 미니언일 때 (최대 체력 비례 대미지)
        if (TargetType == EUnitType::Minion)
        {
            float MaxHP = TargetStat->GetMaxHP();
            int32 ID = TargetStat->GetObjectID();
            float Ratio = 0.f;

            switch (ID)
            {
            case 3001: Ratio = 0.45f; break; // 전사
            case 3002: Ratio = 0.70f; break; // 법사
            case 3003: Ratio = 0.14f; break; // 공성
            case 3004: Ratio = 0.05f; break; // 슈퍼
            }

            if (Ratio > 0.f)
            {
                FinalDamage = FMath::Max(BaseData.Base_AD, MaxHP * Ratio);
            }
        }
        // 2. 챔피언일 때 (가열 시스템)
        else if (TargetType == EUnitType::Champion)
        {
            FinalDamage = BaseData.Base_AD * GetCurrentHeatingMultiplier();
        }
    }

    return FinalDamage;
}

float UObjectStatComponent::GetCurrentHeatingMultiplier() const
{
    return 1.0f + (CurrentHeatStack * MechData.Heating_Rate);
}

void UObjectStatComponent::AddHeatingStack()
{
    if (MechData.Max_Heating > 0)
        CurrentHeatStack = FMath::Min(CurrentHeatStack + 1, MechData.Max_Heating);
}

void UObjectStatComponent::ResetHeatingStack()
{
    CurrentHeatStack = 0;
}

void UObjectStatComponent::ApplyDamage(float Amount)
{
    if (IsDead()) return;
    float DamageMultiplier = 100.0f / (100.0f + CurrentArmor);
    CurrentHP = FMath::Clamp(CurrentHP - (Amount * DamageMultiplier), 0.f, GetMaxHP());
    OnHPChanged.Broadcast(CurrentHP, GetMaxHP());
}

void UObjectStatComponent::OnRep_CurrentHP()
{
    OnHPChanged.Broadcast(CurrentHP, GetMaxHP());
}