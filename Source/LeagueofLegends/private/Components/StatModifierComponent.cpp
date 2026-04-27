// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/StatModifierComponent.h"

#include "LeagueofLegends.h"


UStatModifierComponent::UStatModifierComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FStatModifierHandle UStatModifierComponent::AddModifier(const FStatModifier& Modifier)
{
	FStatModifierHandle Handle;
	Handle.ID = NextHandleID++;

	Modifiers.Add(Handle.ID, Modifier);

	return Handle;
}

void UStatModifierComponent::RemoveModifier(const FStatModifierHandle& Handle)
{
	if (!Handle.IsValid())
	{
		PRINTLOG_TK(TEXT("Invalid StatModifierHandle: %d"), Handle.ID);
		return;
	}

	const FStatModifier* Found = Modifiers.Find(Handle.ID);
	if (!Found)
	{
		PRINTLOG_TK(TEXT("StatModifierHandle not found: %d"), Handle.ID);
		return;
	}
	
	ELolStatType StatType = Found->StatType;
	Modifiers.Remove(Handle.ID);
}

float UStatModifierComponent::GetFinalValue(ELolStatType StatType, float BaseValue) const
{
	float AddSum = 0.f;
	float MulProduct = 1.f;
	
	for (const auto& Pair : Modifiers)
	{
		const FStatModifier& Mod = Pair.Value;
		if (Mod.StatType == StatType)
		{
			if (Mod.Op == EModifierOp::Add)
			{
				AddSum += Mod.Value;
			}
			else
			{
				MulProduct *= Mod.Value;
			}
		}
	}
	
	return (BaseValue + AddSum) * MulProduct;
}


