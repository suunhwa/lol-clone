// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WidgetViewBase.h"
#include "ShopWidget.generated.h"

struct FItemProfileViewData;
class UItemProfileWidget;
class UTextBlock;
class UButton;
class UWrapBox;
/**
 * 
 */
UCLASS()
class LEAGUEOFLEGENDS_API UShopWidget : public UWidgetViewBase
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
public:
	virtual void BindViewModel(UViewModelBase* InViewModel) override;
	virtual void UnbindViewModel() override;
	
private:	
	void PopulateItemList(const TArray<FItemProfileViewData>& ViewData);
	void OnGoldUpdated(int32 NewGold);
	
	UFUNCTION()
	void OnItemSlotClicked(int32 ItemID);

	UFUNCTION()
	void OnPurchaseClicked();

	UFUNCTION()
	void OnSellClicked();

	UFUNCTION()
	void OnUndoClicked();
	
	UFUNCTION()
	void OnCloseClicked();

private:
	UPROPERTY(VisibleAnywhere)
	int32 SelectedItemID = INDEX_NONE;
	
	UPROPERTY(VisibleAnywhere)
	int32 SelectedSlotIndex = INDEX_NONE;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UItemProfileWidget> ItemProfileWidgetClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> WrapBox_ItemList;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Purchase;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Sell;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Undo;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_CurrentGold;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Close;
};
