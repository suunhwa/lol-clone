#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Type/RiftTypes.h"
#include "LoLChampionAnimInstance.generated.h"

class ALoLCharacterBase;
class UStateComponent;
class UStatComponent;

UCLASS()
class LEAGUEOFLEGENDS_API ULoLChampionAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
	// AnimBP에서 읽는 값들
	UPROPERTY(BlueprintReadOnly, Category = "State")
	float MoveSpeed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsMoving = false;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsAttacking = false;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsCastingSkill = false;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsHit = false;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsDead = false;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	ECharacterState CurrentState = ECharacterState::Idle;

private:
	UPROPERTY()
	TObjectPtr<ALoLCharacterBase> OwnerChar;

	UPROPERTY()
	TObjectPtr<UStateComponent> StateComp;
};
