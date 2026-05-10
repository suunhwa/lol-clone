// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WidgetViewBase.h"
#include "InventoryWidget.generated.h"

class UTextBlock;
class UScaleBox;
/**
 * 
 */
UCLASS()
class LEAGUEOFLEGENDS_API UInventoryWidget : public UWidgetViewBase
{
	GENERATED_BODY()
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScaleBox> ScaleBox_Slot0;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScaleBox> ScaleBox_Slot1;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScaleBox> ScaleBox_Slot2;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScaleBox> ScaleBox_Slot3;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScaleBox> ScaleBox_Slot4;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScaleBox> ScaleBox_Slot5;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_CurrentGold;
};
