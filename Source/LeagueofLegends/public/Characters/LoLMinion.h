#pragma once

#include "CoreMinimal.h"
#include "Characters/LoLCharacterBase.h"
#include "LoLMinion.generated.h"

class AAStarGridManager;

UCLASS()
class LEAGUEOFLEGENDS_API ALoLMinion : public ALoLCharacterBase
{
	GENERATED_BODY()

public:
	ALoLMinion();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// --- 데이터 및 스탯 ---
	UPROPERTY(EditAnywhere, Category = "Minion | Data")
	int32 MinionID; 

	float CachedAttackRange = 0.f;
	float CachedAttackSpeed = 1.0f;

	// --- AI 및 우선순위 ---
	void UpdateAggro();
	AActor* ScanForBestTarget();
	int32 GetTargetPriority(AActor* PotentialTarget);

	// --- A* 길찾기 및 이동 ---
	void RequestNewPath(FVector Destination);
	void MoveAlongPath(float DeltaTime);

private:
	UPROPERTY()
	AAStarGridManager* GridManager;

	TArray<FVector> CurrentPath;
	int32 CurrentPathIndex = 0;
    
	TWeakObjectPtr<AActor> CurrentTarget;
    

	float PathUpdateTimer = 0.f;
	const float PathUpdateInterval = 0.5f; // A* 계산 최적화 주기
};

	/*UPROPERTY(VisibleAnywhere, Category = "Minion|Base")
	FString Name_KR;
    
	// --- [GrowthTable 관련 변수 - 실시간 계산용] ---
	UPROPERTY(VisibleAnywhere, Category = "Minion|Stats")
	float HP; // 현재/최대 체력

	UPROPERTY(VisibleAnywhere, Category = "Minion|Stats")
	float AttackDamage;*/

	// float LastAttackTime = 0.0f;
	
/*public:
	// 부모의 ReceiveDamage를 오버라이드하여 미니언 전용 HP 로직과 연결
	virtual void ReceiveDamage_Implementation(float Amount, EDamageType DamageType, AActor* DamageInstigator) override;

	// 포탑이 팀을 확인할 때 사용할 인터페이스 함수 오버라이드
	virtual ETeam GetTeam_Implementation() const override;
	
};*/


