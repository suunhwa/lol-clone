// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CooldownComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LEAGUEOFLEGENDS_API UCooldownComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCooldownComponent();

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

public:
	void StartCooldown(FName Tag, float Duration);
	bool IsOnCooldown(FName Tag) const;
	float GetRemaining(FName Tag) const;

	// LoL CDR 상한 45%
	void SetCDR(float InCDR) { CDR = FMath::Clamp(InCDR, 0.f, 0.45f); }

private:
	TMap<FName, float> Cooldowns;

	float CDR = 0.f;

	float ApplyCDR(float BaseDuration) const { return BaseDuration * (1.f - CDR); }
};
