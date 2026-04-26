// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/TargetingComponent.h"
#include "Components/TagComponent.h"
#include "Components/StatComponent.h"

UTargetingComponent::UTargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UTargetingComponent::IsValidTarget(AActor* Attacker) const
{
	if (!Attacker) return false;

	UTagComponent* MyTag = GetOwner()->FindComponentByClass<UTagComponent>();
	UTagComponent* AttackerTag = Attacker->FindComponentByClass<UTagComponent>();
	UStatComponent* MyStat = GetOwner()->FindComponentByClass<UStatComponent>();

	if (!MyTag || !AttackerTag || !MyStat) return false;

	if (MyStat->IsDead()) return false;
	if (MyTag->HasTag(UnitTags::Dead)) return false;
	if (MyTag->HasTag(UnitTags::Untargetable)) return false;
	if (!MyTag->IsEnemy(AttackerTag)) return false;

	// TODO: FOW 시야 체크 추가

	return true;
}

int32 UTargetingComponent::GetPriority(AActor* Attacker) const
{
	UTagComponent* MyTag = GetOwner()->FindComponentByClass<UTagComponent>();
	if (!MyTag) return 0;

	// 유닛 타입별 기본 우선순위
	// 타워/미니언 AI에서 타겟 선택 시 사용
	switch (MyTag->GetUnitType())
	{
	case EUnitType::Champion: return 3;
	case EUnitType::Tower: return 2;
	case EUnitType::Minion: return 1;
	default: return 0;
	}
}
