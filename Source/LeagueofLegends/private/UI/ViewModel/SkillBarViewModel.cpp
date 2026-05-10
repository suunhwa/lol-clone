#include "UI/ViewModel/SkillBarViewModel.h"

#include "Characters/LoLChampion.h"
#include "Characters/Data/ChampionData.h"
#include "Components/CooldownComponent.h"
#include "Components/SkillComponent.h"
#include "Components/StatComponent.h"

void USkillBarViewModel::Setup(ALoLChampion* InChampion)
{
	Champion = InChampion;
	if (!Champion || !Champion->StatComp) { return; }

	Champion->StatComp->OnHPChanged.AddUObject(this, &USkillBarViewModel::HandleHPChanged);
	Champion->StatComp->OnManaChanged.AddUObject(this, &USkillBarViewModel::HandleManaChanged);

	HandleHPChanged(Champion->StatComp->GetCurrentHP(), Champion->StatComp->GetMaxHP());
	HandleManaChanged(Champion->StatComp->GetCurrentMana(), Champion->StatComp->GetMaxMana());
}

UCooldownComponent* USkillBarViewModel::GetCooldownComp() const
{
	return Champion ? Champion->CooldownComp : nullptr;
}

UChampionData* USkillBarViewModel::GetChampionData() const
{
	return Champion ? Champion->GetChampionData() : nullptr;
}

UStatComponent* USkillBarViewModel::GetStatComp() const
{
	return Champion ? Champion->StatComp : nullptr;
}

USkillComponent* USkillBarViewModel::GetSkillComp() const
{
	return Champion ? Champion->SkillComp : nullptr;
}

void USkillBarViewModel::HandleHPChanged(float Current, float Max)
{
	OnHPChanged.Broadcast(Current, Max);
}

void USkillBarViewModel::HandleManaChanged(float Current, float Max)
{
	OnManaChanged.Broadcast(Current, Max);
}
