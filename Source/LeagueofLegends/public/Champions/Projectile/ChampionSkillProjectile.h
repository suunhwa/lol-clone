// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/CombatComponent.h"
#include "ChampionSkillProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraSystem;
class UNiagaraComponent;

// 발사체 피격 시 추가 처리를 위한 델리게이트 (W 고리 적용, 평타 마크 소비 등)
DECLARE_DELEGATE_OneParam(FOnProjHit, AActor*)

UCLASS(Blueprintable)
class LEAGUEOFLEGENDS_API AChampionSkillProjectile : public AActor
{
	GENERATED_BODY()

public:
	AChampionSkillProjectile();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void Launch(FDamageContext InCtx, float Speed, float MaxRange, bool bPiercing, bool bInCooldownOnHit = false);
	void SetCollisionRadius(float Radius);

	// Debug trail 폭 (R=160, Q/E/W=0)
	UPROPERTY()
	float DebugTrailHalfWidth = 0.f;

	// 발사 속도 — 클라이언트 ProjectileMovement 시뮬레이션용
	UPROPERTY(ReplicatedUsing = OnRep_LaunchVelocity)
	FVector LaunchVelocity = FVector::ZeroVector;

	UFUNCTION()
	void OnRep_LaunchVelocity();

	// 클라이언트 비주얼 이펙트 — OnRep로 수신 즉시 스폰
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedVFX)
	TObjectPtr<UNiagaraSystem> ReplicatedVFX;

	UPROPERTY(Replicated)
	FVector ReplicatedVFXScale = FVector(1.f);

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> VFXComp;

	UFUNCTION()
	void OnRep_ReplicatedVFX();

	void SpawnReplicatedVFX();

	void SetReplicatedVFX(UNiagaraSystem* VFX, FVector Scale = FVector(1.f))
	{
		ReplicatedVFX = VFX;
		ReplicatedVFXScale = Scale;
	}

	// true이면 구조물(타워/억제기/넥서스)에 피해를 주지 않음
	bool bCanDamageStructures = true;

	// 피격 시 추가 콜백 (W 고리 적용, 평타 마크 소비 등). ApplyHit 후 호출됨
	FOnProjHit OnHitDelegate;

private:
	FVector PrevLocation = FVector::ZeroVector;

protected:
	// 관통 발사체 (W): Overlap 방식
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* Overlapped, AActor* Other,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	// 비관통 발사체 (Q, E): Hit 방식
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* Other,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	void ApplyHit(AActor* Target);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> CollisionComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	FDamageContext DamageCtx;
	bool bIsPiercing = false;
	bool bCooldownOnHit = false;
	TSet<TWeakObjectPtr<AActor>> HitActors; // 관통 시 중복 타격 방지
};
