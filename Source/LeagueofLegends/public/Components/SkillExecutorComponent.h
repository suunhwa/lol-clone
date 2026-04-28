// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/CombatComponent.h"
#include "Components/SkillComponent.h"
#include "SkillExecutorComponent.generated.h"

class ALoLCharacterBase;
class AChampionSkillProjectile;
class UStatComponent;
class UCombatComponent;
class UCooldownComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LEAGUEOFLEGENDS_API USkillExecutorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USkillExecutorComponent();

protected:
	virtual void BeginPlay() override;

public:
	// LoLChampion이 OnSkillActivated 수신 시 호출
	virtual void Execute(ESkillSlot Slot, FVector TargetLoc) {}

	// 발사체 BP 클래스. 에디터(BP_EzrealSkillExecutor 등)에서 챔피언별 설정
	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	TSubclassOf<AChampionSkillProjectile> ProjectileClass;

protected:
	// 모든 클라이언트에 몽타주 재생
	void PlayMontage(UAnimMontage* Montage) const;

	// 발사체 스폰 (서버 전용)
	AChampionSkillProjectile* SpawnProjectile(
		const FVector& Direction, float Speed, float Range,
		FDamageContext Ctx, bool bPiercing, bool bCooldownOnHit = false) const;

	// 오너 컴포넌트 캐시 (BeginPlay에서 세팅)
	UPROPERTY()
	TObjectPtr<ALoLCharacterBase> OwnerChar;

	UPROPERTY()
	TObjectPtr<UStatComponent> StatComp;

	UPROPERTY()
	TObjectPtr<UCombatComponent> CombatComp;

	UPROPERTY()
	TObjectPtr<UCooldownComponent> CooldownComp;
};
