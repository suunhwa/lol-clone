// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemEffectRegistry.h"

#include "LeagueofLegends.h"

TSubclassOf<UItemPassiveEffectBase> UItemEffectRegistry::FindPassiveClass(const FName& EffectName) const
{
	const TSubclassOf<UItemPassiveEffectBase>* Found = PassiveMap.Find(EffectName);
	
	if (!Found)
	{
		PRINTLOG_TK(TEXT("Passive effect not found: %s"), *EffectName.ToString());
		return nullptr;
	}
	
	return *Found;
}
