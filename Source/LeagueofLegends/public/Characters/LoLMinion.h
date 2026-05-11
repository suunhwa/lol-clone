#pragma once

#include "CoreMinimal.h"
#include "Characters/LoLCharacterBase.h"
#include "Interfaces/Targetable.h"

#include "LoLMinion.generated.h"

class AAStarGridManager;

UCLASS()
class LEAGUEOFLEGENDS_API ALoLMinion : public ALoLCharacterBase
{
	GENERATED_BODY()

public:
	ALoLMinion();

	// --- ITargetable Interface 구현 ---
	virtual ETeam GetTeam_Implementation() const override;
	virtual EUnitType GetUnitType_Implementation() const override;
	virtual AActor* GetCurrentCombatTarget_Implementation() const override;
	// ---------------------------------
	
	// --- ISightProvider Interface 구현 ---
	virtual bool IsHideable_Implementation() const override { return true; }
	
	// 스포너가 소환 직후 호출해줄 함수
	void SetLanePath(const TArray<FVector>& InPath) { LanePath = InPath; CurrentLaneIndex = 0; }
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnDeath(AActor* DamageInstigator) override;
	
	// --- AI 설정 ---
	UPROPERTY(EditAnywhere, Category = "Minion | AI")
	float AcquisitionRange = 600.f;  // 어그로 인식 범위

	UPROPERTY(EditAnywhere, Category = "Minion | AI")
	float LoseTargetRange = 1000.f;  // 어그로 해제 범위

	// --- 데이터 및 스탯 ---
	UPROPERTY(EditAnywhere, Category = "Minion | Data")
	int32 MinionID; 

	float CachedAttackRange = 0.f;
	float CachedAttackSpeed = 1.0f;

	// --- AI 및 우선순위 ---
	void UpdateAggro(float DeltaTime);
	AActor* ScanForBestTarget();
	int32 GetTargetPriority(AActor* PotentialTarget);

	// --- A* 길찾기 및 이동 ---
	void RequestNewPath(FVector Destination);
	void MoveAlongPath(float DeltaTime);

	TWeakObjectPtr<AActor> CurrentTarget;
	
private:
	UPROPERTY()
	AAStarGridManager* GridManager;

	TArray<FVector> CurrentPath;
	int32 CurrentPathIndex = 0;
    
	
	
	float PathUpdateTimer = 0.f;
	const float PathUpdateInterval = 0.5f; // A* 계산 최적화 주기
	
	float AggroUpdateTimer = 0.f;
	const float AggroUpdateInterval = 0.3f; // 0.3초마다 타겟 갱신
	
	UPROPERTY()
	TArray<FVector> LanePath; // 스포너가 준 체크포인트 리스트

	int32 CurrentLaneIndex = 0; // 현재 가야 할 체크포인트 번호
	
protected:
	// 마지막 공격 시간 저장 (공격 속도 기반 쿨타임 계산용)
	float LastAttackTime = 0.f;

	// 언리얼 기본 데미지 전달 함수 오버라이드
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// 사망 처리 함수
	void Die(AActor* Killer);

	// 공격 속도를 쿨타임(초)으로 변환하는 헬퍼
	float GetAttackCooldown() const;
	
	// 실제 공격 실행 함수
	virtual void ExecuteAttack();
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


