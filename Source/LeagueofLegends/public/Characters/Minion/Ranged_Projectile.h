#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Ranged_Projectile.generated.h"

UCLASS()
class LEAGUEOFLEGENDS_API ALoLRanged_Projectile : public AActor
{
	GENERATED_BODY()

public:
	ALoLRanged_Projectile();

	// 발사 시 초기화 함수
	void Launch(float InSpeed, float InDamage, FName InTeamTag);

protected:
	// 무언가에 부딪혔을 때 호출될 함수
	UFUNCTION()
	void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UStaticMeshComponent* MeshComp;

private:
	float Damage;
	FName OwnerTeamTag;
};