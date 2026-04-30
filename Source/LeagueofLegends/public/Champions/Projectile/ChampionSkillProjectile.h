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

	// bPiercing=true: 관통 (W), false: 첫 적 명중 후 소멸 (Q, E 보조)
	// bCooldownOnHit: 명중 시 시전자 쿨다운 1.5초 감소 (이즈리얼 Q 패시브)
	void Launch(FDamageContext InCtx, float Speed, float MaxRange, bool bPiercing, bool bInCooldownOnHit = false);

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
