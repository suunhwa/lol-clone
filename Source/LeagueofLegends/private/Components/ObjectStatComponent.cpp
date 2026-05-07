#include "Components/ObjectStatComponent.h"
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
    OnHPChanged.Broadcast(CurrentHP, GetMaxHP());
}

float UObjectStatComponent::GetAttackDamage() const
{
    return BaseData.Base_AD * GetCurrentHeatingMultiplier();
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