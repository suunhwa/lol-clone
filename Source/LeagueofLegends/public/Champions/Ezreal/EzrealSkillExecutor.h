// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkillExecutorComponent.h"
#include "EzrealSkillExecutor.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class ALoLChampion;

UCLASS(Blueprintable)
class LEAGUEOFLEGENDS_API UEzrealSkillExecutor : public USkillExecutorComponent
{
	GENERATED_BODY()

public:
	UEzrealSkillExecutor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Execute(ESkillSlot Slot, FVector TargetLoc) override;

	virtual void OnBasicAttackFired(AChampionSkillProjectile* Proj, AActor* Target) override;

private:
	void ExecuteQ(FVector TargetLoc);
	void ExecuteW(FVector TargetLoc);
	void ExecuteE(FVector TargetLoc);
	void ExecuteR(FVector TargetLoc);
	void FireESecondaryShot();

	class UChampionDataSubsystem* GetDataSub() const;
	FName GetChampionID() const;
	int32 GetRank(ESkillSlot Slot) const;

	// ── W 고리 (Essence Flux Mark) ──────────────────
	void SetWMark(AActor* Target, float BonusDamage);
	void ClearWMark();
	void OnWProjectileHit(AActor* Target);    // W 발사체 피격 → 마크 적용
	void OnWMarkConsumed(AActor* Target);     // 평타/E 미사일이 마크 소비 → 보너스 딜

	TWeakObjectPtr<AActor> WMarkTarget;
	TWeakObjectPtr<AChampionSkillProjectile> WProjectile;
	float WMarkBonusDamage = 0.f;
	FTimerHandle WMarkExpireTimer;

	// 마크 적중 시 대상 몸에 붙는 이펙트 (4초 후 or 소비 시 제거)
	UPROPERTY(EditDefaultsOnly, Category = "VFX|W")
	TObjectPtr<UNiagaraSystem> W_MarkEffect;

	// 챔피언 기준(반지름 42) 대비 이펙트 크기 배율 — 에디터에서 보면서 조정
	UPROPERTY(EditDefaultsOnly, Category = "VFX|W", meta = (ClampMin = "0.01", ClampMax = "5.0"))
	float W_MarkEffectScale = 0.3f;

	// 챔피언용 추가 Z 오프셋 (캡슐 중심 기준)
	UPROPERTY(EditDefaultsOnly, Category = "VFX|W")
	float W_MarkEffectZOffset_Champion = 0.f;

	// 타워용 추가 Z 오프셋 (바운딩박스 중심 기준)
	UPROPERTY(EditDefaultsOnly, Category = "VFX|W")
	float W_MarkEffectZOffset_Tower = 0.f;

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> WMarkEffectComp;
	
	// ── VFX ──────────────────────────────────────
	// Q: 발사 시 머즐 이펙트
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Q")
	TObjectPtr<UNiagaraSystem> Q_MuzzleEffect;

	// W: 발사 시 머즐 이펙트
	UPROPERTY(EditDefaultsOnly, Category = "VFX|W")
	TObjectPtr<UNiagaraSystem> W_MuzzleEffect;

	// E: 블링크 출발지 이펙트
	UPROPERTY(EditDefaultsOnly, Category = "VFX|E")
	TObjectPtr<UNiagaraSystem> E_DepartEffect;

	// E: 블링크 도착지 이펙트
	UPROPERTY(EditDefaultsOnly, Category = "VFX|E")
	TObjectPtr<UNiagaraSystem> E_ArriveEffect;

	// R: 차지 중 이펙트
	UPROPERTY(EditDefaultsOnly, Category = "VFX|R")
	TObjectPtr<UNiagaraSystem> R_ChargeEffect;

	// R: 발사 시 머즐 이펙트
	UPROPERTY(EditDefaultsOnly, Category = "VFX|R")
	TObjectPtr<UNiagaraSystem> R_MuzzleEffect;

	// ── SFX ──────────────────────────────────────
	UPROPERTY(EditDefaultsOnly, Category = "SFX")
	TObjectPtr<USoundBase> Q_CastSound;

	UPROPERTY(EditDefaultsOnly, Category = "SFX")
	TObjectPtr<USoundBase> W_CastSound;

	UPROPERTY(EditDefaultsOnly, Category = "SFX")
	TObjectPtr<USoundBase> E_BlinkSound;

	UPROPERTY(EditDefaultsOnly, Category = "SFX")
	TObjectPtr<USoundBase> R_CastSound;

	// ChampionData 몽타주 접근용 (OwnerChar보다 구체적인 타입)
	UPROPERTY()
	TObjectPtr<ALoLChampion> OwnerChampion;
};
