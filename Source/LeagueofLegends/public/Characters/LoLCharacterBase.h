// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/Damageable.h"
#include "Interfaces/Targetable.h"
#include "LoLCharacterBase.generated.h"

class UStatComponent;
class UCombatComponent;
class UTagComponent;
class UStateComponent;
class UStatusEffectComponent;
class UCooldownComponent;
class USkillComponent;
class UTargetingComponent;
class UWidgetComponent;

UCLASS(Abstract)
class LEAGUEOFLEGENDS_API ALoLCharacterBase : public ACharacter, public IDamageable, public ITargetable
{
	GENERATED_BODY()

public:
	ALoLCharacterBase();

protected:
	virtual void BeginPlay() override;

public:
	// --- IDamageable
	virtual void ReceiveDamage(float Amount, EDamageType DamageType, AActor* DamageInstigator) override;
	virtual bool IsDead() const override;

	// --- ITargetable
	virtual bool IsTargetable() const override;
	virtual FVector GetTargetLocation() const override;
	virtual ETeam GetTeam() const override;

	// --- Components
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStatComponent> StatComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UCombatComponent> CombatComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UTagComponent> TagComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStateComponent> StateComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStatusEffectComponent> StatusEffectComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UCooldownComponent> CooldownComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USkillComponent> SkillComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UTargetingComponent> TargetingComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UWidgetComponent> HPBarWidgetComp;

	UPROPERTY(EditAnywhere, Category = "Team")
	ETeam InitialTeam = ETeam::Blue;

	// 모든 클라이언트에 몽타주 재생 (서버에서 호출)
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayMontage(UAnimMontage* Montage);

protected:
	virtual void OnDeath(AActor* DamageInstigator);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnDeath();
};
