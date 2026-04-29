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

	// 실제 투사체를 발사하거나 데미지를 입히는 함수
	void Fire();

	// 현재 타겟
	UPROPERTY(VisibleAnywhere, Category = "AI")
	AActor* CurrentTarget;

	// 공격 타이머 핸들
	FTimerHandle AttackTimerHandle;

	// --- 스탯 (깡통) ---
	UPROPERTY(EditAnywhere, Category = "Stats")
	float AttackDamage = 150.f;

	UPROPERTY(EditAnywhere, Category = "Stats")
	float AttackRange = 850.f;

	UPROPERTY(EditAnywhere, Category = "Stats")
	float AttackInterval = 1.2f; // 초당 공격 횟수의 역수
};