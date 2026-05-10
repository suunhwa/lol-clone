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

UCLASS()
class LEAGUEOFLEGENDS_API ALoLChampion : public ALoLCharacterBase
{
	GENERATED_BODY()

public:
	ALoLChampion();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	// 서버가 캐릭터 선택 후 ChampionDataSubsystem에서 받아 세팅.
	// EditDefaultsOnly는 테스트 전용 — 실제론 SetChampionData() 사용
	UPROPERTY(ReplicatedUsing = OnRep_ChampionData, EditDefaultsOnly, BlueprintReadOnly, Category = "Champion")
	TObjectPtr<UChampionData> ChampionData;

public:
	virtual void Tick(float DeltaTime) override;
	
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

	// 리스폰 (서버 전용)
	void Respawn();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Respawn();


protected:
	virtual void OnDeath(AActor* DamageInstigator) override;

private:
	void CreateSkillExecutor();
	void HandleSkillActivated(ESkillSlot Slot, FVector TargetLoc);
	void AttackLoopTick();

	UFUNCTION()
	void OnRep_ChampionData();

private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStatModifierComponent> StatModifierComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UInventoryComponent> InventoryComp;

	// 동적 생성되는 챔피언별 스킬 실행 컴포넌트
	UPROPERTY()
	TObjectPtr<USkillExecutorComponent> SkillExecutor;

	TWeakObjectPtr<AActor> AttackTarget;
	FTimerHandle AttackLoopTimer;
	FTimerHandle BasicAttackImpactTimer;
	FTimerHandle RespawnTimer;
	int32 AttackSectionIndex = 0;

	UPROPERTY(EditAnywhere, Category = "Respawn")
	float RespawnDelay = 5.f;
};
