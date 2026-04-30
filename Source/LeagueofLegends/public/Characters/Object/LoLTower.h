#pragma once

#include "CoreMinimal.h"
#include "Characters/LoLStructure.h"
#include "LoLTower.generated.h"

UCLASS()
class LEAGUEOFLEGENDS_API ALoLTower : public ALoLStructure
{
	GENERATED_BODY()

public:
	ALoLTower();

protected:
	virtual void BeginPlay() override;

	// 타겟을 찾고 공격을 결정하는 메인 로직
	void CheckAndAttack();

	// 실제 데미지를 입히고 시각화하는 함수
	void Fire();

	// 현재 타겟
	UPROPERTY(VisibleAnywhere, Category = "Object|AI")
	TObjectPtr<AActor> CurrentTarget;

	// 공격 타이머 핸들
	FTimerHandle AttackTimerHandle;

	// --- 런타임 적용 스탯 ---
	float AttackDamage;
	float AttackRange;
	float AttackInterval;

	// [추가] 가열(Heating) 시스템용 변수
	int32 CurrentHeatStack = 0;
};