#include "UI/ViewModel/PlayerStatsViewModel.h"

#include "Characters/LoLChampion.h"
#include "Components/StatComponent.h"
#include "Components/InventoryComponent.h"

void UPlayerStatsViewModel::Setup(ALoLChampion* InChampion)
{
	Champion = InChampion;
	if (!Champion) { return; }

	if (Champion->StatComp)
	{
		Champion->StatComp->OnLevelChanged.AddUObject(this, &UPlayerStatsViewModel::HandleLevelChanged);
	}

	if (UInventoryComponent* InvComp = Champion->GetInventoryComp())
	{
		InvComp->OnInventorySlotChanged.AddUObject(this, &UPlayerStatsViewModel::HandleInventoryChanged);
	}
}

float UPlayerStatsViewModel::GetAD() const
{
	return Champion && Champion->StatComp ? Champion->StatComp->GetAD() : 0.f;
}

float UPlayerStatsViewModel::GetAP() const
{
	return Champion && Champion->StatComp ? Champion->StatComp->GetAP() : 0.f;
}

float UPlayerStatsViewModel::GetArmor() const
{
	return Champion && Champion->StatComp ? Champion->StatComp->GetArmor() : 0.f;
}

float UPlayerStatsViewModel::GetMR() const
{
	return Champion && Champion->StatComp ? Champion->StatComp->GetMagicResist() : 0.f;
}

float UPlayerStatsViewModel::GetAS() const
{
	return Champion && Champion->StatComp ? Champion->StatComp->GetAttackSpeed() : 0.f;
}

float UPlayerStatsViewModel::GetAH() const
{
	return Champion && Champion->StatComp ? Champion->StatComp->GetAbilityHaste() : 0.f;
}

float UPlayerStatsViewModel::GetCrit() const
{
	return Champion && Champion->StatComp ? Champion->StatComp->GetCritChance() * 100.f : 0.f;
}

float UPlayerStatsViewModel::GetMS() const
{
	return Champion && Champion->StatComp ? Champion->StatComp->GetMoveSpeed() : 0.f;
}

void UPlayerStatsViewModel::HandleLevelChanged(int32 /*NewLevel*/)
{
	OnStatsRefresh.Broadcast();
}

void UPlayerStatsViewModel::HandleInventoryChanged(int32 /*SlotIndex*/, UItemInstance* /*Item*/)
{
	OnStatsRefresh.Broadcast();
}
