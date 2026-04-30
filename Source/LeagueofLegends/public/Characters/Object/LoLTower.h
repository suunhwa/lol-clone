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

	// 타워 AI 메인 루틴: 타겟 유효성 검사 및 탐색
	void CheckAndAttack();

	// 실제 대미지 연산 및 시각화
	void Fire();

	// 현재 공격 중인 대상
	UPROPERTY(VisibleAnywhere, Category = "Object|AI")
	TObjectPtr<AActor> CurrentTarget;

	// 공격 타이머 핸들
	FTimerHandle AttackTimerHandle;

	// 현재 적용 중인 가열 스택
	UPROPERTY(VisibleAnywhere, Category = "Object|Stats")
	int32 CurrentHeatStack = 0;

	// 마지막으로 공격한 대상 (대상이 바뀌면 스택 초기화를 위해 필요)
	UPROPERTY()
	TObjectPtr<AActor> LastAttackedTarget;
};