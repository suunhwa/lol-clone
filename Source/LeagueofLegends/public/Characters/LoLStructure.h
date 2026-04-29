#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Damageable.h" // 인터페이스 포함
#include "Interfaces/Targetable.h"     // ETeam 정의 위치에 따라 수정 필요
#include "LoLStructure.generated.h"

UCLASS()
class LEAGUEOFLEGENDS_API ALoLStructure : public AActor, public IDamageable, public ITargetable
{
	GENERATED_BODY()

public:
	ALoLStructure();

protected:
	virtual void BeginPlay() override;

public:
	// --- IDamageable Interface ---
	virtual void ReceiveDamage(float Amount, EDamageType DamageType, AActor* DamageInstigator) override;
	virtual bool IsDead() const override { return bIsDestroyed; }

	// --- ITargetable Interface ---
	virtual bool IsTargetable() const override { return !bIsDestroyed; }
	virtual FVector GetTargetLocation() const override { return GetActorLocation(); }
	virtual ETeam GetTeam() const override { return Team; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Health = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	ETeam Team;

	bool bIsDestroyed = false;

	virtual void OnDestroyed();
};