// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/CooldownComponent.h"

UCooldownComponent::UCooldownComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCooldownComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (auto It = Cooldowns.CreateIterator(); It; ++It)
	{
		It.Value() -= DeltaTime;
		if (It.Value() <= 0.f)
			It.RemoveCurrent();
	}
}

void UCooldownComponent::StartCooldown(FName Tag, float Duration)
{
	Cooldowns.Add(Tag, ApplyCDR(Duration));
}

bool UCooldownComponent::IsOnCooldown(FName Tag) const
{
	const float* Remaining = Cooldowns.Find(Tag);
	return Remaining && *Remaining > 0.f;
}

float UCooldownComponent::GetRemaining(FName Tag) const
{
	const float* Remaining = Cooldowns.Find(Tag);
	return Remaining ? *Remaining : 0.f;
}
