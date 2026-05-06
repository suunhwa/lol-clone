// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemDataAsset.h"
#include "UObject/Object.h"
#include "ItemPassiveEffectBase.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class LEAGUEOFLEGENDS_API UItemPassiveEffectBase : public UObject
{
	GENERATED_BODY()
	
public:
	UItemPassiveEffectBase();
	
	virtual void InitializeEffect(AActor* InOwner, const FItemPassiveEffectData& InEffectData);
	
	virtual void OnEquipped();
	
	virtual void OnUnequipped();
	
protected:
	UPROPERTY(VisibleAnywhere, Category = "Effect")
	TWeakObjectPtr<AActor> OwnerChampion;

	// 효과 실행에 필요한 수치 데이터 복사본
	UPROPERTY(VisibleAnywhere, Category = "Effect")
	FItemPassiveEffectData EffectData;

	// 현재 효과가 적용 중인지 확인하기 위한 플래그
	UPROPERTY(VisibleAnywhere, Category = "Effect")
	bool bIsActive;
};
