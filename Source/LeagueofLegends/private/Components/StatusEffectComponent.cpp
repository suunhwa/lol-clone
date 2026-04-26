// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/StatusEffectComponent.h"
#include "Components/TagComponent.h"

static FName GetTagForEffect(EStatusEffect Type)
{
	switch (Type)
	{
	case EStatusEffect::Stun:    return UnitTags::Stunned;
	case EStatusEffect::Root:    return UnitTags::Rooted;
	case EStatusEffect::Silence: return UnitTags::Silenced;
	case EStatusEffect::Knockup: return UnitTags::Knockup;
	default:                     return NAME_None;
	}
}

UStatusEffectComponent::UStatusEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UStatusEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner()->HasAuthority()) return;

	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		ActiveEffects[i].Remaining -= DeltaTime;
		if (ActiveEffects[i].Remaining <= 0.f)
			RemoveEffect(ActiveEffects[i].Type);
	}
}

void UStatusEffectComponent::ApplyEffect(EStatusEffect Type, float Duration, float Magnitude)
{
	for (FActiveStatusEffect& Effect : ActiveEffects)
	{
		if (Effect.Type == Type)
		{
			Effect.Remaining = FMath::Max(Effect.Remaining, Duration);
			Effect.Duration  = FMath::Max(Effect.Duration, Duration);
			Effect.Magnitude = FMath::Max(Effect.Magnitude, Magnitude);
			return;
		}
	}

	FActiveStatusEffect New;
	New.Type      = Type;
	New.Duration  = Duration;
	New.Remaining = Duration;
	New.Magnitude = Magnitude;
	ActiveEffects.Add(New);

	FName Tag = GetTagForEffect(Type);
	if (Tag != NAME_None)
	{
		UTagComponent* TagComp = GetOwner()->FindComponentByClass<UTagComponent>();
		if (TagComp) TagComp->AddTag(Tag);
	}
}

void UStatusEffectComponent::RemoveEffect(EStatusEffect Type)
{
	ActiveEffects.RemoveAll([Type](const FActiveStatusEffect& E) { return E.Type == Type; });

	FName Tag = GetTagForEffect(Type);
	if (Tag != NAME_None)
	{
		UTagComponent* TagComp = GetOwner()->FindComponentByClass<UTagComponent>();
		if (TagComp) TagComp->RemoveTag(Tag);
	}
}

void UStatusEffectComponent::RemoveAll()
{
	for (const FActiveStatusEffect& Effect : ActiveEffects)
	{
		FName Tag = GetTagForEffect(Effect.Type);
		if (Tag != NAME_None)
		{
			UTagComponent* TagComp = GetOwner()->FindComponentByClass<UTagComponent>();
			if (TagComp) TagComp->RemoveTag(Tag);
		}
	}
	ActiveEffects.Empty();
}

bool UStatusEffectComponent::HasEffect(EStatusEffect Type) const
{
	return ActiveEffects.ContainsByPredicate([Type](const FActiveStatusEffect& E) { return E.Type == Type; });
}

float UStatusEffectComponent::GetSlowMagnitude() const
{
	float Max = 0.f;
	for (const FActiveStatusEffect& E : ActiveEffects)
	{
		if (E.Type == EStatusEffect::Slow)
			Max = FMath::Max(Max, E.Magnitude);
	}
	return Max;
}

bool UStatusEffectComponent::CanMove() const
{
	UTagComponent* TagComp = GetOwner()->FindComponentByClass<UTagComponent>();
	if (!TagComp) return true;
	return !TagComp->HasTag(UnitTags::Stunned)
		&& !TagComp->HasTag(UnitTags::Rooted)
		&& !TagComp->HasTag(UnitTags::Knockup);
}

bool UStatusEffectComponent::CanAttack() const
{
	UTagComponent* TagComp = GetOwner()->FindComponentByClass<UTagComponent>();
	if (!TagComp) return true;
	return !TagComp->HasTag(UnitTags::Stunned)
		&& !TagComp->HasTag(UnitTags::Knockup);
}

bool UStatusEffectComponent::CanCastSkill() const
{
	UTagComponent* TagComp = GetOwner()->FindComponentByClass<UTagComponent>();
	if (!TagComp) return true;
	return !TagComp->HasTag(UnitTags::Stunned)
		&& !TagComp->HasTag(UnitTags::Silenced)
		&& !TagComp->HasTag(UnitTags::Knockup);
}
