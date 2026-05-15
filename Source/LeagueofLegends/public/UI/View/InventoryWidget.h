// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WidgetViewBase.h"
#include "InventoryWidget.generated.h"

struct FInventorySlotViewData;
class UItemSlotWidget;
class UTextBlock;
class UScaleBox;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnInventorySlotSelected, int32);

/**
 * 
 */
UCLASS()
class LEAGUEOFLEGENDS_API UInventoryWidget : public UWidgetViewBase
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
public:
	virtual void BindViewModel(UViewModelBase* InViewModel) override;
	virtual void UnbindViewModel() override;

	FOnInventorySlotSelected OnInventorySlotSelected;
	
private:
	void HandleSlotChanged(const FInventorySlotViewData& SlotData);
	void HandleGoldChanged(int32 NewGold);
	void HandleInventorySlotClicked(int32 SlotIndex);
	
	void InitSlots();
	
private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UItemSlotWidget> ItemSlotWidgetClass;
	
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
	
	TArray<TObjectPtr<UItemSlotWidget>> SlotWidgets;
};
