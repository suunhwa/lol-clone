// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/StateComponent.h"
#include "Components/TagComponent.h"
#include "Net/UnrealNetwork.h"

UStateComponent::UStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UStateComponent, CurrentState);
}

bool UStateComponent::TryChangeState(ECharacterState NewState)
{
	if (CurrentState == NewState) { return false; }
	if (!CanTransitionTo(NewState)) { return false; }

	PreviousState = CurrentState;
	CurrentState = NewState;
	OnStateChanged.Broadcast(PreviousState, CurrentState);
	return true;
}

bool UStateComponent::CanTransitionTo(ECharacterState NewState) const
{
	// Dead → Idle만 허용 (리스폰)
	if (CurrentState == ECharacterState::Dead)
	{
		return NewState == ECharacterState::Idle;
	}


	if (NewState == ECharacterState::Dead) { return true; }
	if (NewState == ECharacterState::Hit) { return true; }

	if (CurrentState == ECharacterState::CastingSkill && NewState == ECharacterState::BasicAttacking) { return false; }
	if (CurrentState == ECharacterState::BasicAttacking && NewState == ECharacterState::CastingSkill) { return false; }

	UTagComponent* TagComp = GetOwner()->FindComponentByClass<UTagComponent>();
	if (!TagComp) { return true; }

	switch (NewState)
	{
	case ECharacterState::Moving:
		return !TagComp->HasTag(UnitTags::Stunned)
			&& !TagComp->HasTag(UnitTags::Rooted)
			&& !TagComp->HasTag(UnitTags::Knockup);
	case ECharacterState::BasicAttacking:
		return !TagComp->HasTag(UnitTags::Stunned)
			&& !TagComp->HasTag(UnitTags::Knockup);
	case ECharacterState::CastingSkill:
		return !TagComp->HasTag(UnitTags::Stunned)
			&& !TagComp->HasTag(UnitTags::Silenced)
			&& !TagComp->HasTag(UnitTags::Knockup);
	default:
		return true;
	}
}

void UStateComponent::OnRep_CurrentState()
{
	OnStateChanged.Broadcast(PreviousState, CurrentState);
	PreviousState = CurrentState;
}
