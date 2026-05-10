// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/SkillComponent.h"

#include "LeagueofLegends.h"
#include "Components/CooldownComponent.h"
#include "Components/StatComponent.h"
#include "Components/TagComponent.h"
#include "Components/StateComponent.h"

USkillComponent::USkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USkillComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	CooldownComp = Owner->FindComponentByClass<UCooldownComponent>();
	StatComp = Owner->FindComponentByClass<UStatComponent>();
	TagComp = Owner->FindComponentByClass<UTagComponent>();
	StateComp = Owner->FindComponentByClass<UStateComponent>();
}

bool USkillComponent::RequestActivateSkill(ESkillSlot Slot, FVector TargetLocation)
{
	if (!GetOwner()->HasAuthority())
	{
		PRINTLOG_SH(TEXT("SKILLCOMP: NO AUTHORITY"));
		return false;
	}
	
	if (GetRank(Slot) == 0)
	{
		PRINTLOG_SH(TEXT("SKILLCOMP: RANK 0"));
		return false;
	}

	// TODO: DataAsset 연결 후 실제 마나 코스트 전달
	if (!CanActivate(Slot, 0.f))
	{
		PRINTLOG_SH(TEXT("SKILLCOMP: CanActivate false - CD:%d Mana:%d Tag:%d State:%d"), CooldownComp == nullptr, StatComp == nullptr, TagComp == nullptr, StateComp == nullptr); 
		return false;
	}

	OnSkillActivated.Broadcast(Slot, TargetLocation);
	return true;
}

bool USkillComponent::CanActivate(ESkillSlot Slot, float ManaCost) const
{
	if (!CooldownComp || !StatComp || !TagComp || !StateComp) return false;

	if (CooldownComp->IsOnCooldown(GetCooldownTag(Slot))) return false;
	if (StatComp->GetCurrentMana() < ManaCost) return false;
	if (TagComp->HasTag(UnitTags::Stunned)) return false;
	if (TagComp->HasTag(UnitTags::Silenced)) return false;
	if (TagComp->HasTag(UnitTags::Knockup)) return false;
	if (!StateComp->IsAlive()) return false;

	return true;
}

bool USkillComponent::AssignSkillPoint(ESkillSlot Slot)
{
	uint8 Idx = static_cast<uint8>(Slot);
	if (Ranks[Idx] >= MaxRanks[Idx]) return false;
	Ranks[Idx]++;
	OnRankChanged.Broadcast();
	return true;
}

void USkillComponent::ApplySkillPointClient(ESkillSlot Slot)
{
	uint8 Idx = static_cast<uint8>(Slot);
	if (Ranks[Idx] < MaxRanks[Idx])
		Ranks[Idx]++;
	OnRankChanged.Broadcast();
}

bool USkillComponent::IsMaxRank(ESkillSlot Slot) const
{
	uint8 Idx = static_cast<uint8>(Slot);
	return Ranks[Idx] >= MaxRanks[Idx];
}

FName USkillComponent::GetCooldownTag(ESkillSlot Slot) const
{
	switch (Slot)
	{
	case ESkillSlot::Q: return TEXT("Skill.Q");
	case ESkillSlot::W: return TEXT("Skill.W");
	case ESkillSlot::E: return TEXT("Skill.E");
	case ESkillSlot::R: return TEXT("Skill.R");
	default: return NAME_None;
	}
}
