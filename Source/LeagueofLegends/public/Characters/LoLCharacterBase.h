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

	virtual AActor* GetCurrentCombatTarget_Implementation() const override;
public:
	ALoLCharacterBase();

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	// [FacingRotation 제거] SetReplicateMovement(true)가 회전 복제를 이미 처리하므로 중복.
	// SetActorRotation() sweep이 CMC와 충돌하여 상대 캐릭터 공중 부유 유발.
	virtual void FaceRotation(FRotator NewControlRotation, float DeltaTime = 0.f) override;
	virtual void OnRep_PlayerState() override;

public:
	// --- IDamageable
	virtual void ReceiveDamage_Implementation(float Amount, EDamageType DamageType, AActor* DamageInstigator) override;
	virtual bool IsDead_Implementation() const override;

	// --- ITargetable
	virtual bool IsTargetable_Implementation() const override;
	virtual FVector GetTargetLocation_Implementation() const override;
	virtual ETeam GetTeam_Implementation() const override;
	virtual EUnitType GetUnitType_Implementation() const override;
	
	// --- ISightProvider
#pragma region SightProvider
	virtual FVector GetSightOrigin_Implementation() const override;
	virtual float GetSightRange_Implementation() const override;
	virtual bool IsStatic_Implementation() const override;
	virtual ERiftSightTag GetSightTag_Implementation() const override;
	virtual bool IsHideable_Implementation() const override;
	virtual void SetFOWVisibilityFlag_Implementation(ERiftSightTag Team, bool bVisible) override;

private:
	// 비트 플래그: bit0 = Red팀에게 보임, bit1 = Blue팀에게 보임
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_FOWVisibility)
	uint8 FOWVisibilityFlags = 0;
	
public:
	UFUNCTION()
	void OnRep_FOWVisibility();
	
	void ApplyVisibility(bool bVisible);
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Sight")
	float SightRange = 1200.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sight")
	bool bStaticSight = false;
	
	// UPROPERTY(EditDefaultsOnly, Category = "Sight")
	// ERiftSightTag SightTag = ERiftSightTag::None;
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

	/*// TODO : 이거 없어져야 하는 값 같음.
	UPROPERTY(EditAnywhere, Category = "Team")
	ETeam InitialTeam = ETeam::Blue;*/

	// 모든 클라이언트에 몽타주 재생 (서버에서 호출)
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayMontage(UAnimMontage* Montage);

	// 클라이언트 쿨타임 동기화
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartCooldown(FName Tag, float Duration);

	// 특정 섹션부터 몽타주 재생
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayMontageSection(UAnimMontage* Montage, FName SectionName);

	// FacingRotation 완전 제거 — SetReplicateMovement(true)가 회전 복제를 처리
	// SetActorRotation + CMC 충돌로 부유/회전 이상 발생하므로 사용 안 함

protected:
	virtual void OnDeath(AActor* DamageInstigator);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnDeath();

public:
	void RefreshHUDDisplay(); // 팀 색상 + 닉네임만 갱신 (델리게이트 재구독 없음)

private:
	void InitPlayerHUDWidget();
	
private:
	// 💰 롤 스타일 골드 / XP 플로팅 텍스트 위젯 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> FloatingTextWidgetComp;

public:
	// 📣 순수 C++로만 작동하는 플로팅 텍스트 출력 함수 (블루프린트 그래프 필요 없음)
	UFUNCTION(Client, Reliable)
	void Client_CreateFloatingText(int32 Amount, bool bIsGold, FVector SpawnLocation);
	void Client_CreateFloatingText_Implementation(int32 Amount, bool bIsGold, FVector SpawnLocation);
};
