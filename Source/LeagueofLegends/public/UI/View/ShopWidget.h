// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WidgetViewBase.h"
#include "ShopWidget.generated.h"

class UDescStatWidget;
class UVerticalBox;
class UImage;
class UCanvasPanel;
struct FItemProfileViewData;
class UItemProfileWidget;
class UTextBlock;
class UButton;
class UWrapBox;
class UItemDataAsset;
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
	
	void SetSelectedSlotIndex(int32 InSlotIndex);
	
	void SetCanvasPanelHitTestInvisible();
	
	void SetItemDescription(const UItemDataAsset* ItemData);
	void ResetItemDescription();
	
private:
	void PopulateItemList(const TArray<FItemProfileViewData>& ViewData);
	void OnGoldUpdated(int32 NewGold);
	
	void OnItemSlotClicked(int32 ItemID, UItemProfileWidget* ClickedItem);

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
	
	UPROPERTY()
	UItemProfileWidget* CurrentSelectedItem = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UItemProfileWidget> ItemProfileWidgetClass;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> RootCanvas;

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
		
	// ------------------- Description -------------------
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDescStatWidget> DescStatWidgetClass;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UItemProfileWidget> WBP_Desc_ItemProfile;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_Desc_ItemIcon;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Desc_ItemName;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Desc_ItemPrice;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VBox_Desc_ItemStatList;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Desc_ItemDescription;
};






























