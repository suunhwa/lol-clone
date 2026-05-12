// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemIconRegistry.generated.h"

/**
 * 
 */
UCLASS()
class LEAGUEOFLEGENDS_API UItemIconRegistry : public UDataAsset
{
	GENERATED_BODY()
	
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TMap<FName, TObjectPtr<UTexture2D>> IconMap;

	TObjectPtr<UTexture2D> GetIcon(FName ItemID) const
	{
		const TObjectPtr<UTexture2D>* Found = IconMap.Find(ItemID);
		return Found ? *Found : nullptr;
	}
};
