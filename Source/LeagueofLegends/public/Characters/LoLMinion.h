// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/LoLCharacterBase.h"
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
	float HP = 50.0f;

	UPROPERTY(EditAnywhere, Category = "AI|Stats")
	float AttackDamage = 10.0f;

	UPROPERTY(EditAnywhere, Category = "AI|Stats")
	float AttackRange = 150.0f; 

	// 이동속도 // 나중에 데이터테이블에서 가져오기 전까지 사용할 임시값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float MoveSpeed = 300.0f;
	
	UPROPERTY(EditAnywhere, Category = "AI|Stats")
	float AttackSpeed = 1.0f; // 초당 공격 횟수

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


