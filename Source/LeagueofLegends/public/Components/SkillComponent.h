// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SkillComponent.generated.h"

class UCooldownComponent;
class UStatComponent;
class UTagComponent;
class UStateComponent;

UENUM(BlueprintType)
enum class ESkillSlot : uint8
{
	Q = 0,
	W = 1,
	E = 2,
	R = 3,
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LEAGUEOFLEGENDS_API USkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USkillComponent();

protected:
	virtual void BeginPlay() override;

public:
	// 검증 통과 시 OnSkillActivated 브로드캐스트 후 true 반환
	bool RequestActivateSkill(ESkillSlot Slot, FVector TargetLocation);

	bool AssignSkillPoint(ESkillSlot Slot);

	int32 GetRank(ESkillSlot Slot) const { return Ranks[static_cast<uint8>(Slot)]; }
	bool IsMaxRank(ESkillSlot Slot) const;

	// Champion이 구독해서 실제 스킬 로직 실행
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSkillActivated, ESkillSlot, FVector);
	FOnSkillActivated OnSkillActivated;

private:
	int32 Ranks[4] = {0, 0, 0, 0};

	static constexpr int32 MaxRanks[4] = {5, 5, 5, 3};

	bool CanActivate(ESkillSlot Slot, float ManaCost) const;
	FName GetCooldownTag(ESkillSlot Slot) const;

	UCooldownComponent* CooldownComp = nullptr;
	UStatComponent* StatComp = nullptr;
	UTagComponent* TagComp = nullptr;
	UStateComponent* StateComp = nullptr;
};
