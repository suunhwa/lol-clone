// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemPassiveEffectBase.h"

UItemPassiveEffectBase::UItemPassiveEffectBase()
{
	bIsActive = false;
}

void UItemPassiveEffectBase::InitializeEffect(AActor* InOwner, const FItemPassiveEffectData& InEffectData)
{
	if (!InOwner)
	{
		return;
	}
	
	OwnerChampion = InOwner;
	EffectData = InEffectData;
}

void UItemPassiveEffectBase::OnEquipped()
{
	if (OwnerChampion.IsValid())
	{
		bIsActive = true;
	}
}

void UItemPassiveEffectBase::OnUnequipped()
{
	if (OwnerChampion.IsValid())
	{
		bIsActive = false;
	}
}
