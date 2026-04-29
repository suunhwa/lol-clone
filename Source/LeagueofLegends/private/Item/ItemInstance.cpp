// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemInstance.h"
#include "Components/StatModifierComponent.h"
#include "LeagueofLegends.h"

void UItemInstance::Initialize(UItemDataAsset* InDataAsset, AActor* InOwner)
{
	if (!InDataAsset || !InOwner)
	{
		PRINTLOG_TK(TEXT("Invalid DataAsset or Owner"));
		return;
	}
	
	ItemDataAsset = InDataAsset;
	OwnerChampion = InOwner;
}

void UItemInstance::OnEquipped()
{
	if (!ItemDataAsset || !OwnerChampion.IsValid())
	{
		PRINTLOG_TK(TEXT("OnEquipped Failed: Invalid DataAsset or Owner"));
		return;
	}
	
	UStatModifierComponent* StatModifierComp = OwnerChampion->FindComponentByClass<UStatModifierComponent>();
	if (!StatModifierComp)
	{
		PRINTLOG_TK(TEXT("OnEquipped Failed: StatModifierComponent not found"));
		return;
	}
	
	const TArray<FStatModifier>& Modifiers = ItemDataAsset->Stats;
	for (const FStatModifier& Mod : Modifiers)
	{
		FStatModifierHandle Handle = StatModifierComp->AddModifier(Mod);
		RegisteredHandles.Add(Handle);
	}
	
	PRINTLOG_TK(TEXT("[%s] OnEquipped: %d Modifiers registered"), *ItemDataAsset->NameKR, RegisteredHandles.Num());
}

void UItemInstance::OnUnequipped()
{
	if (!OwnerChampion.IsValid())
	{
		PRINTLOG_TK(TEXT("OnUnequipped Failed: Invalid Owner"));
		return;
	}
	
	UStatModifierComponent* StatModifierComp = OwnerChampion->FindComponentByClass<UStatModifierComponent>();
	if (!StatModifierComp)
	{
		PRINTLOG_TK(TEXT("OnUnequipped Failed: StatModifierComponent not found"));
		return;
	}
	
	for (const FStatModifierHandle& Handle : RegisteredHandles)
	{
		StatModifierComp->RemoveModifier(Handle);
	}

	PRINTLOG_TK(TEXT("[%s] OnUnequipped: %d Modifiers removed"), *ItemDataAsset->NameKR, RegisteredHandles.Num());

	RegisteredHandles.Empty();
}
