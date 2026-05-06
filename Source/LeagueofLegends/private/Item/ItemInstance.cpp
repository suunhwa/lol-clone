// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemInstance.h"
#include "Components/StatModifierComponent.h"
#include "LeagueofLegends.h"
#include "Item/ItemPassiveEffectBase.h"

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
	if (StatModifierComp)
	{
		const TArray<FStatModifier>& Modifiers = ItemDataAsset->Stats;
		for (const FStatModifier& Mod : Modifiers)
		{
			FStatModifierHandle Handle = StatModifierComp->AddModifier(Mod);
			RegisteredHandles.Add(Handle);
		}
		PRINTLOG_TK(TEXT("[%s] OnEquipped: %d Modifiers registered"), *ItemDataAsset->NameKR, RegisteredHandles.Num());
	}
	
	for (const FItemPassiveEffectData& EffectData : ItemDataAsset->Effects)
	{
		if (!EffectData.PassiveClass) continue;

		UItemPassiveEffectBase* Passive = NewObject<UItemPassiveEffectBase>(this, EffectData.PassiveClass);
		Passive->InitializeEffect(OwnerChampion.Get(), EffectData);
		Passive->OnEquipped();

		ActivePassives.Add(Passive);
	}
	PRINTLOG_TK(TEXT("[%s] OnEquipped: Passives activated: %d"), *ItemDataAsset->NameKR, ActivePassives.Num());
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
	
	// 스탯 모디파이어 제거
	for (const FStatModifierHandle& Handle : RegisteredHandles)
	{
		StatModifierComp->RemoveModifier(Handle);
	}

	PRINTLOG_TK(TEXT("[%s] OnUnequipped: %d Modifiers removed"), *ItemDataAsset->NameKR, RegisteredHandles.Num());

	RegisteredHandles.Empty();
	
	// 패시브 비활성화
	for (UItemPassiveEffectBase* Passive : ActivePassives)
	{
		if (Passive)
		{
			Passive->OnUnequipped();
		}
	}
	
	PRINTLOG_TK(TEXT("[%s] OnUnequipped: Passives deactivated: %d"), *ItemDataAsset->NameKR, ActivePassives.Num());

	ActivePassives.Empty();
}
