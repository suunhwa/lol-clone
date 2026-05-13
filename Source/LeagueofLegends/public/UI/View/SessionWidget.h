// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WidgetViewBase.h"
#include "Type/RiftTypes.h"
#include "SessionWidget.generated.h"

class UCheckBox;
class UEditableText;
class UButton;
class UTextBlock;
class USessionViewModel;
/**
 * 
 */
UCLASS()
class LEAGUEOFLEGENDS_API USessionWidget : public UWidgetViewBase
{
	GENERATED_BODY()
	
public:
	virtual void BindViewModel(UViewModelBase* InViewModel) override;
	virtual void UnbindViewModel() override;

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> ET_namespace;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> btn_start;

	// 성공/실패 메시지 표시
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> txt_status;
	
	// 게임 모드 버튼
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> btn_mode_rift;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> btn_mode_dodgeball;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> btn_mode_event;

	// 게임 모드 체크박스
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> cbx_mode_rift;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> cbx_mode_dodgeball;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> cbx_mode_event;
	
	// ------------------ button handler ------------------
	UFUNCTION() 
	void OnStartClicked();
	
	UFUNCTION()
	void OnModeRiftClicked();
	
	UFUNCTION() 
	void OnModeDodgeballClicked();
	
	UFUNCTION() 
	void OnModeEventClicked();
	
	// ------------------ checkbox handler ------------------
	UFUNCTION() 
	void OnCbxRiftChanged(bool bIsChecked);
	
	UFUNCTION() 
	void OnCbxDodgeballChanged(bool bIsChecked);
	
	UFUNCTION() 
	void OnCbxEventChanged(bool bIsChecked);
	
	void SelectMode(EMatchMode Mode);

	// ViewModel 콜백
	void OnSessionStatus(bool bSuccess, const FString& Message);
};
