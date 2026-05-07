// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/Damageable.h"
#include "Interfaces/SightProvider.h"
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
class LEAGUEOFLEGENDS_API ALoLCharacterBase : public ACharacter, public IDamageable, public ITargetable, public ISightProvider
{
	GENERATED_BODY()

public:
	ALoLCharacterBase();

protected:
	virtual void BeginPlay() override;

public:
	// --- IDamageable
	virtual void ReceiveDamage_Implementation(float Amount, EDamageType DamageType, AActor* DamageInstigator) override;
	virtual bool IsDead_Implementation() const override;

	// --- ITargetable
	virtual bool IsTargetable_Implementation() const override;
	virtual FVector GetTargetLocation_Implementation() const override;
	virtual ETeam GetTeam_Implementation() const override;
	virtual EUnitType GetUnitType_Implementation() const override;
	virtual AActor* GetCurrentCombatTarget_Implementation() const override;
	
	// --- ISightProvider
#pragma region SightProvider
	virtual FVector GetSightOrigin_Implementation() const override;
	virtual float GetSightRange_Implementation() const override;
	virtual bool IsStatic_Implementation() const override;
	virtual ERiftSightTag GetSightTag_Implementation() const override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Sight")
	float SightRange = 1000.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sight")
	bool bStaticSight = false;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sight")
	ERiftSightTag SightTag = ERiftSightTag::None;
#pragma endregion

public:
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

	// 특정 섹션부터 몽타주 재생
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayMontageSection(UAnimMontage* Montage, FName SectionName);

protected:
	virtual void OnDeath(AActor* DamageInstigator);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnDeath();
};
