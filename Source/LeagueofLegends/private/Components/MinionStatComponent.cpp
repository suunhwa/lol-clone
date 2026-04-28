#include "Components/MinionStatComponent.h"

UMinionStatComponent::UMinionStatComponent()
{
	
}

void UMinionStatComponent::InitMinionStats(const FMinionBaseRow& InBaseRow, const FMinionGrowthRow& InGrowthRow, int32 GameMinutes)
{
	// 1. 공통 기본 스탯 (BaseRow)
	BaseMoveSpeed = InBaseRow.MoveSpeed;
	BaseAS = InBaseRow.AtkSpeed;
	BaseRange = InBaseRow.AtkRange;
	BaseArmor = (float)InBaseRow.Armor;
	BaseMR = (float)InBaseRow.MR;
    
	// 미니언 특수 속성
	TowerDamageReduction = InBaseRow.Tower_DR;
	bIsSiege = InBaseRow.Is_Siege;
	bIsSuper = InBaseRow.Is_Super;

	// 2. 성장 스탯 계산 (GrowthRow)
    
	int32 CycleCount = 0;
	if (InGrowthRow.Interval > 0)
	{
		CycleCount = FMath::FloorToInt((float)GameMinutes / InGrowthRow.Interval);
		if (InGrowthRow.Max_Cycle > 0)
		{
			CycleCount = FMath::Min(CycleCount, InGrowthRow.Max_Cycle);
		}
	}

	// 초기값 세팅
	BaseHP = InGrowthRow.Base_HP + (InGrowthRow.HP_Up * CycleCount);
	BaseAD = InGrowthRow.Base_AD + (InGrowthRow.AD_Up * CycleCount);
    
	// 골드 보상 계산 로직 넣으면 좋을거 같음
	BaseGoldReward = InGrowthRow.Base_Gold + FMath::FloorToInt(InGrowthRow.Gold_Up * (float)CycleCount);
	
	// 3. 상태 적용
	CurrentHP = BaseHP;

	UE_LOG(LogTemp, Log, TEXT("Minion Initialized: %s (HP: %.f, AD: %.1f, Gold: %d)"), 
		*InBaseRow.Name_KR, BaseHP, BaseAD, BaseGoldReward);
}