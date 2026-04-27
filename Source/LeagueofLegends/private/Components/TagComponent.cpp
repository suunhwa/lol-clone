// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/TagComponent.h"
#include "Net/UnrealNetwork.h"

UTagComponent::UTagComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UTagComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UTagComponent, Team);
	DOREPLIFETIME(UTagComponent, UnitType);
	DOREPLIFETIME(UTagComponent, Tags);
}

bool UTagComponent::IsEnemy(const UTagComponent* Other) const
{
	if (!Other) return false;
	if (Team == ETeam::None || Other->Team == ETeam::None) return false;
	return Team != Other->Team;
}

bool UTagComponent::IsAlly(const UTagComponent* Other) const
{
	if (!Other) return false;
	return Team == Other->Team;
}

void UTagComponent::AddTag(FName Tag)
{
	if (Tags.Contains(Tag)) return;
	Tags.Add(Tag);
	OnTagChanged.Broadcast(Tag, true);
}

void UTagComponent::RemoveTag(FName Tag)
{
	if (!Tags.Contains(Tag)) return;
	Tags.Remove(Tag);
	OnTagChanged.Broadcast(Tag, false);
}

bool UTagComponent::HasTag(FName Tag) const
{
	return Tags.Contains(Tag);
}

void UTagComponent::OnRep_Tags()
{
	// 클라이언트: 태그 변경 브로드캐스트 (애니/UI 갱신용)
}
