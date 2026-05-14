#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LeagueofLegends.h" // ETeam 정의가 포함된 헤더
#include "Type/RiftTypes.h"
#include "NiagaraComponent.h"

#include "Ranged_Projectile.generated.h"

UCLASS()
class LEAGUEOFLEGENDS_API ALoLRanged_Projectile : public AActor
{
	GENERATED_BODY()

public:
	ALoLRanged_Projectile();

	// 발사 설정 (OwnerTeam 추가)
	void Launch(AActor* InTarget, float InSpeed, float InDamage, ETeam InOwnerTeam);

protected:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UStaticMeshComponent* MeshComp;

private:
	float Damage;
	ETeam OwnerTeam; // 팀 정보 저장
	TWeakObjectPtr<AActor> TargetActor;
};