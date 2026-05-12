// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/View/SlotWidgetBase.h"
#include "ItemSlotWidget.generated.h"

class UItemInstance;
class UButton;
class UOverlay;


DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemInventorySlotClicked, int32);
UCLASS()
class LEAGUEOFLEGENDS_API UItemSlotWidget : public USlotWidgetBase
{
	GENERATED_BODY()
	
public:
	void SetItemIcon(UTexture2D* Icon);
	void SetSlotIndex(int32 InSlotIndex);

	FOnItemInventorySlotClicked OnSlotClicked;

	void BindSlotButtonClick();

private:
	UFUNCTION()
	void HandleSlotButtonClicked();
	
private:
	int32 SlotIndex;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Icon;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> Overlay_CD;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Stack;
};

