// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ChampionData.generated.h"

/**
 * 
 */
UCLASS()
class LEAGUEOFLEGENDS_API UChampionData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Champion|Info")
	FName ChampionID;

	UPROPERTY(EditDefaultsOnly, Category = "Champion|Visual")
	TObjectPtr<USkeletalMesh> Mesh;

	UPROPERTY(EditDefaultsOnly, Category = "Champion|Visual")
	TObjectPtr<UAnimInstance> AnimBP;

	UPROPERTY(EditDefaultsOnly, Category = "Champion|Animation")
	TObjectPtr<UAnimMontage> BasicAttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Champion|Animation")
	TObjectPtr<UAnimMontage> QSkillMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Champion|Animation")
	TObjectPtr<UAnimMontage> WSkillMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Champion|Animation")
	TObjectPtr<UAnimMontage> ESkillMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Champion|Animation")
	TObjectPtr<UAnimMontage> RSkillMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Champion|Animation")
	TObjectPtr<UAnimMontage> HitMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Champion|Animation")
	TObjectPtr<UAnimMontage> DeathMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = "Champion|Animation")
	TObjectPtr<UAnimMontage> RespawnMontage;

	// 수치는 DataTable에서 ChampionID로 조회
	// FChampionBaseRow (MoveSpeed 등)
	UPROPERTY(EditDefaultsOnly, Category = "Champion|Data")
	TObjectPtr<UDataTable> BaseTable;
	
	// FChampionStatRow (HP, AD 등)
	UPROPERTY(EditDefaultsOnly, Category = "Champion|Data")
	TObjectPtr<UDataTable> StatTable; 
	
	// FChampionGrowthRow
	UPROPERTY(EditDefaultsOnly, Category = "Champion|Data")
	TObjectPtr<UDataTable> GrowthTable;
	
	// FMasterSkillCoreRow
	UPROPERTY(EditDefaultsOnly, Category = "Champion|Data")
	TObjectPtr<UDataTable> SkillCoreTable; 
	
	// FDetailSkillStatsRow
	UPROPERTY(EditDefaultsOnly, Category = "Champion|Data")
	TObjectPtr<UDataTable> SkillDetailTable; 
};
