// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/StatModifierTypes.h"
#include "StatModifierComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LEAGUEOFLEGENDS_API UStatModifierComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UStatModifierComponent();

	FStatModifierHandle AddModifier(const FStatModifier& Modifier);
	
	void RemoveModifier(const FStatModifierHandle& Handle);
	
	float GetFinalValue(ELolStatType StatType, float BaseValue) const;
	
private:
	int32 NextHandleID = 0;
	
	TMap<int32, FStatModifier> Modifiers;
};
