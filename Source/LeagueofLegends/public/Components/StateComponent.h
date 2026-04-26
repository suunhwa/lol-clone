// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/RiftTypes.h"
#include "StateComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnStateChanged, ECharacterState /*OldState*/, ECharacterState /*NewState*/);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LEAGUEOFLEGENDS_API UStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStateComponent();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// 상태 전환
	// StatusEffectComp가 막으면 실패
	bool TryChangeState(ECharacterState NewState);

	ECharacterState GetCurrentState() const { return CurrentState; }
	bool IsAlive() const { return CurrentState != ECharacterState::Dead; }

	FOnStateChanged OnStateChanged;

private:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentState)
	ECharacterState CurrentState = ECharacterState::Idle;

	bool CanTransitionTo(ECharacterState NewState) const;

	UFUNCTION()
	void OnRep_CurrentState();

	ECharacterState PreviousState = ECharacterState::Idle;
};
