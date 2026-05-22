// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/LoLCharacterBase.h"
#include "Components/SkillComponent.h"
#include "Components/SkillExecutorComponent.h"
#include "Data/ChampionData.h"

#include "LoLChampion.generated.h"

class UInventoryComponent;
class UStatModifierComponent;
class UDecalComponent;

UCLASS()
class LEAGUEOFLEGENDS_API ALoLChampion : public ALoLCharacterBase
{
	GENERATED_BODY()

public:
	ALoLChampion();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PawnClientRestart() override;
	virtual void PostNetReceiveLocationAndRotation() override;

	// 서버가 캐릭터 선택 후 ChampionDataSubsystem에서 받아 세팅.
	// EditDefaultsOnly는 테스트 전용 — 실제론 SetChampionData() 사용
	UPROPERTY(ReplicatedUsing = OnRep_ChampionData, EditDefaultsOnly, BlueprintReadOnly, Category = "Champion")
	TObjectPtr<UChampionData> ChampionData;

public:
	virtual void Tick(float DeltaTime) override;
	virtual bool IsHideable_Implementation() const override { return true; } // 챔피언은 은신 불가능 (예외: 쉔 W)
	
public:
#pragma region Component Getters
	UStatModifierComponent* GetStatModifierComp() const { return StatModifierComp; }
	UInventoryComponent* GetInventoryComp() const { return InventoryComp; }
#pragma endregion
	

	// 서버에서 챔피언 선택 완료 후 호출 (GameMode → 서버 전용)
	void SetChampionData(UChampionData* Data);

	UChampionData* GetChampionData() const { return ChampionData; }

	// 평타 실행 (서버에서 호출)
	void ExecuteBasicAttack(AActor* Target);

	// 공격 루프 (서버 전용)
	void StartAttackLoop(AActor* Target);
	void StopAttackLoop();

	// 이동 시스템 (서버 권위 + 로컬 예측)
	void SetMoveTarget(FVector Destination);
	void SetMoveTargetFrom(FVector StartLoc, FVector Destination);  // 서버/클라 동일 경로 보장
	void CancelMove();

	// 리스폰 (서버 전용)
	void Respawn();
	void StartRespawnTimer(float Delay);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Respawn();

	// 귀환 (클라이언트 → Server_RequestRecall → StartRecall)
	void StartRecall();
	void CancelRecall();

	UFUNCTION(Server, Reliable)
	void Server_RequestRecall();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_CancelRecall();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayRecallLanding();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ShowRecallDecal();

	
	// 동적 생성되는 챔피언별 스킬 실행 컴포넌트
	UPROPERTY()
	TObjectPtr<USkillExecutorComponent> SkillExecutor;
	
	// AnimBP용 이동 속도 — 서버에서 세팅, 복제로 모든 클라이언트에 전달
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Animation")
	FVector AnimVelocity = FVector::ZeroVector;

protected:
	virtual void OnDeath(AActor* DamageInstigator) override;
	virtual void OnDamageReceived() override;
	
private:
	void CreateSkillExecutor();
	void HandleSkillActivated(ESkillSlot Slot, FVector TargetLoc);
	void AttackLoopTick();
	void UpdateMovement(float DeltaTime);
	void CompleteRecall();

	UFUNCTION()
	void OnRep_ChampionData();

private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStatModifierComponent> StatModifierComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UInventoryComponent> InventoryComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UDecalComponent> RecallGroundComp;



	TWeakObjectPtr<AActor> AttackTarget;
	FTimerHandle AttackLoopTimer;
	FTimerHandle BasicAttackImpactTimer;
	FTimerHandle RespawnTimer;
	FTimerHandle RecallTimer;
	bool bIsRecalling = false;
	
	int32 AttackSectionIndex = 0;

	// NavPath 이동 상태
	FVector MoveDestination = FVector::ZeroVector;
	TArray<FVector> PathPoints;
	int32 PathIndex = 0;
	bool bHasMoveTarget = false;
	static constexpr float MoveAcceptanceRadius = 80.f;

};
