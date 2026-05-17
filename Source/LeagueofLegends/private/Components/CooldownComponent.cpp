// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/CooldownComponent.h"
#include "Characters/LoLCharacterBase.h"

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
	const float FinalDuration = ApplyCDR(Duration);
	Cooldowns.Add(Tag, FinalDuration);
	Durations.Add(Tag, FinalDuration); // 총 쿨타임 저장

	// 클라이언트에 쿨타임 시작 전파 (서버에서만 호출)
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (ALoLCharacterBase* Char = Cast<ALoLCharacterBase>(GetOwner()))
			Char->Multicast_StartCooldown(Tag, FinalDuration);
	}
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

float UCooldownComponent::GetDuration(FName Tag) const
{
	const float* Duration = Durations.Find(Tag);
	return Duration ? *Duration : 1.f;
}

void UCooldownComponent::ReduceAllCooldowns(float Amount)
{
	for (auto& [Tag, Remaining] : Cooldowns)
		Remaining = FMath::Max(0.f, Remaining - Amount);
}
