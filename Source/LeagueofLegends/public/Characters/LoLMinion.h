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

UCLASS()
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
	FName MinionDataID = TEXT("3002"); 

	// 3. 스탯을 테이블에서 가져와 초기화하는 함수 선언
	void InitializeStatsFromTable();
	
	
	
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
	
	// --- 전투 스탯 (임시) ---
	UPROPERTY(EditAnywhere, Category = "AI|Stats")
	float HP;

	UPROPERTY(EditAnywhere, Category = "AI|Stats")
	float AttackDamage;

	UPROPERTY(EditAnywhere, Category = "AI|Stats")
	float AttackRange; 

	// 이동속도 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float MoveSpeed;
	
	UPROPERTY(EditAnywhere, Category = "AI|Stats")
	float AttackSpeed; // 초당 공격 횟수

	float LastAttackTime = 0.0f;
	
public:
	// 타겟 갱신 함수
	UFUNCTION()
	void UpdateTarget();

	UFUNCTION()
	void PerformAttack();
	
	UFUNCTION()
	void MoveAlongPath(float DeltaTime);
};


