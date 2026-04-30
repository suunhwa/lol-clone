// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/LoLCharacterBase.h"
#include "Struct/MinionStruct.h"
#include "Manager/MinionDataSubsystem.h"
#include "LoLMinion.generated.h"


UENUM(BlueprintType)
enum class EMinionState : uint8
{
	MoveToTarget,    // 타겟(적/넥서스)을 향해 이동
	Attacking,       // 사거리 내에서 공격 중
	Dead             // 사망
};

UCLASS(Abstract)
class LEAGUEOFLEGENDS_API ALoLMinion : public ALoLCharacterBase
{
	GENERATED_BODY()

public:
	ALoLMinion();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	
	// 데미지 처리 함수
	void TakeDamageSimple(float Damage);
	
protected:
	
	// 2. 이 미니언이 어떤 데이터(Row)를 쓸지 결정하는 ID
	// 에디터 디테일 창에서 "Melee", "Ranged" 등을 적어줄 수 있게 합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Data")
	FName MinionDataID; 

	// 3. 스탯을 테이블에서 가져와 초기화하는 함수 선언
	void InitializeStatsFromTable();
	
	UFUNCTION()
	virtual void PerformAttack();
	
	UFUNCTION()
	void MoveAlongPath(float DeltaTime);
	
	// 타겟 갱신 함수
	UFUNCTION()
	void UpdateTarget();
	
	// 플레이어를 타겟으로 저장
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	AActor* TargetPlayer;
	
	UPROPERTY(VisibleAnywhere, Category = "AI")
	EMinionState CurrentState = EMinionState::MoveToTarget;
	
	// A*로 찾아낸 경로 지점들
	TArray<FVector> CurrentPath;
	// 타겟 업데이트 타이머 핸들
	FTimerHandle TargetUpdateTimerHandle;

	UPROPERTY()
	class AAStarGridManager* GridManager;
	
	// 여기서부터 미니언 관련테이블 및 구조체에 있는 모든 변수 선언
	UPROPERTY(VisibleAnywhere, Category = "Minion|Base")
	int32 MinionID;

	UPROPERTY(VisibleAnywhere, Category = "Minion|Base")
	float MoveSpeed;

	UPROPERTY(VisibleAnywhere, Category = "Minion|Base")
	float AttackRange;

	UPROPERTY(VisibleAnywhere, Category = "Minion|Base")
	float AttackSpeed;

	UPROPERTY(VisibleAnywhere, Category = "Minion|Base")
	float ProjSpeed;

	UPROPERTY(VisibleAnywhere, Category = "Minion|Base")
	int32 Armor;

	UPROPERTY(VisibleAnywhere, Category = "Minion|Base")
	int32 MagicResistance;

	UPROPERTY(VisibleAnywhere, Category = "Minion|Base")
	float CollisionRadius;
    
	UPROPERTY(VisibleAnywhere, Category = "Minion|Base")
	bool bIsSiege;

	UPROPERTY(VisibleAnywhere, Category = "Minion|Base")
	bool bIsSuper;

	UPROPERTY(VisibleAnywhere, Category = "Minion|Base")
	float TowerDamageReduction; // Tower_DR
    
	UPROPERTY(VisibleAnywhere, Category = "Minion|Base")
	FString Name_KR;
    
	// --- [GrowthTable 관련 변수 - 실시간 계산용] ---
	UPROPERTY(VisibleAnywhere, Category = "Minion|Stats")
	float HP; // 현재/최대 체력

	UPROPERTY(VisibleAnywhere, Category = "Minion|Stats")
	float AttackDamage;

	float LastAttackTime = 0.0f;
	
public:
	// 부모의 ReceiveDamage를 오버라이드하여 미니언 전용 HP 로직과 연결
	virtual void ReceiveDamage(float Amount, EDamageType DamageType, AActor* DamageInstigator) override;

	// 포탑이 팀을 확인할 때 사용할 인터페이스 함수 오버라이드
	virtual ETeam GetTeam() const override;
};


