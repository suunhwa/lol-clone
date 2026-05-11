// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/CombatComponent.h"
#include "ChampionSkillProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS(Blueprintable)
class LEAGUEOFLEGENDS_API AChampionSkillProjectile : public AActor
{
	GENERATED_BODY()

public:
	AChampionSkillProjectile();

	virtual void Tick(float DeltaTime) override;

	void Launch(FDamageContext InCtx, float Speed, float MaxRange, bool bPiercing, bool bInCooldownOnHit = false);
	void SetCollisionRadius(float Radius);

	// Debug trail 폭 (R=160, Q/E/W=0)
	UPROPERTY()
	float DebugTrailHalfWidth = 0.f;

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
