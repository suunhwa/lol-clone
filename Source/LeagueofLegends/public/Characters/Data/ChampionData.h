// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

class USkillExecutorComponent;

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
	TObjectPtr<UTexture2D> PortraitTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Champion|Skill|Icon")
	TObjectPtr<UTexture2D> PassiveIcon;

	UPROPERTY(EditDefaultsOnly, Category = "Champion|Skill|Icon")
	TObjectPtr<UTexture2D> QIcon;

	UPROPERTY(EditDefaultsOnly, Category = "Champion|Skill|Icon")
	TObjectPtr<UTexture2D> WIcon;

	UPROPERTY(EditDefaultsOnly, Category = "Champion|Skill|Icon")
	TObjectPtr<UTexture2D> EIcon;

	UPROPERTY(EditDefaultsOnly, Category = "Champion|Skill|Icon")
	TObjectPtr<UTexture2D> RIcon;

	UPROPERTY(EditDefaultsOnly, Category = "Champion|Visual")
	TObjectPtr<USkeletalMesh> Mesh;

	UPROPERTY(EditDefaultsOnly, Category = "Champion|Visual")
	TSubclassOf<UAnimInstance> AnimBP;

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

	// 스킬 실행 컴포넌트 클래스. 챔피언별 BP 자식 클래스 지정 (예: BP_EzrealSkillExecutor)
	// 발사체 클래스는 SkillExecutorComponent 안에서 설정
	UPROPERTY(EditDefaultsOnly, Category = "Champion|Skill")
	TSubclassOf<USkillExecutorComponent> SkillExecutorClass;

	// 수치 데이터는 ChampionDataSubsystem이 공용 DataTable에서 ChampionID로 조회
	// BaseTable / StatTable / GrowthTable → ChampionDataSubsystem으로 이전됨
};
