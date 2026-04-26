// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/RiftTypes.h"
#include "TagComponent.generated.h"

namespace UnitTags
{
	static const FName Stunned = TEXT("Stunned");
	static const FName Rooted = TEXT("Rooted");
	static const FName Silenced = TEXT("Silenced");
	static const FName Knockup = TEXT("Knockup");
	static const FName Dead = TEXT("Dead");
	static const FName Untargetable = TEXT("Untargetable");
}

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnTagChanged, FName /*Tag*/, bool /*bAdded*/);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LEAGUEOFLEGENDS_API UTagComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTagComponent();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// --- 팀 / 유닛 타입
	void SetTeam(ETeam InTeam) { Team = InTeam; }
	void SetUnitType(EUnitType InType) { UnitType = InType; }

	ETeam GetTeam() const { return Team; }
	EUnitType GetUnitType() const { return UnitType; }

	bool IsEnemy(const UTagComponent* Other) const;
	bool IsAlly(const UTagComponent* Other) const;

	// --- 일반 태그 (CC, 상태 플래그) 
	void AddTag(FName Tag);
	void RemoveTag(FName Tag);
	bool HasTag(FName Tag) const;

	FOnTagChanged OnTagChanged;

private:
	UPROPERTY(Replicated)
	ETeam Team = ETeam::None;

	UPROPERTY(Replicated)
	EUnitType UnitType = EUnitType::Champion;

	UPROPERTY(ReplicatedUsing = OnRep_Tags)
	TArray<FName> Tags;

	UFUNCTION()
	void OnRep_Tags();
};
