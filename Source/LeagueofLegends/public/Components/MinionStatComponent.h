#pragma once

#include "CoreMinimal.h"
#include "Components/StatComponent.h"
#include "Struct/MinionStruct.h" 
#include "MinionStatComponent.generated.h"

UCLASS()
class LEAGUEOFLEGENDS_API UMinionStatComponent : public UStatComponent
{
	GENERATED_BODY()

public:
	UMinionStatComponent();
	
	void InitMinionStats(const FMinionBaseRow& InBaseRow, const FMinionGrowthRow& InGrowthRow, int32 GameMinutes = 0);

	// 미니언 전용 Getter (구조체에만 있는 특수 값들)
	float GetTowerDamageReduction() const { return TowerDamageReduction; }
	bool IsSiegeMinion() const { return bIsSiege; }
	bool IsSuperMinion() const { return bIsSuper; }
	
protected:
	// 미니언 고유 특성 저장용
	UPROPERTY(VisibleAnywhere, Category = "Stats|Minion")
	float TowerDamageReduction = 0.f;

	UPROPERTY(VisibleAnywhere, Category = "Stats|Minion")
	bool bIsSiege = false;

	UPROPERTY(VisibleAnywhere, Category = "Stats|Minion")
	bool bIsSuper = false;

	// 미니언 보상 정보
	UPROPERTY(VisibleAnywhere, Category = "Stats|Minion")
	int32 BaseGoldReward = 0;
};