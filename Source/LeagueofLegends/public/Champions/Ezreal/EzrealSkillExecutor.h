// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkillExecutorComponent.h"
#include "EzrealSkillExecutor.generated.h"

class ALoLChampion;

UCLASS(Blueprintable)
class LEAGUEOFLEGENDS_API UEzrealSkillExecutor : public USkillExecutorComponent
{
	GENERATED_BODY()

public:
	UEzrealSkillExecutor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Execute(ESkillSlot Slot, FVector TargetLoc) override;

private:
	void ExecuteQ(FVector TargetLoc);
	void ExecuteW(FVector TargetLoc);
	void ExecuteE(FVector TargetLoc);
	void FireESecondaryShot();

	class UChampionDataSubsystem* GetDataSub() const;
	FName GetChampionID() const;
	int32 GetRank(ESkillSlot Slot) const;

	// ChampionData 몽타주 접근용 (OwnerChar보다 구체적인 타입)
	UPROPERTY()
	TObjectPtr<ALoLChampion> OwnerChampion;
};
