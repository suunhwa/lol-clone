// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WidgetViewBase.h"
#include "Type/RiftTypes.h"
#include "GameFramework/LoLSessionSubsystem.h"
#include "SessionWidget.generated.h"

class UCheckBox;
class UEditableText;
class UButton;
class UTextBlock;
class UWidgetSwitcher;
class UScrollBox;
class UVerticalBox;
class USessionViewModel;
class URoomInfoWidget;

UCLASS()
class LEAGUEOFLEGENDS_API USessionWidget : public UWidgetViewBase
{
	GENERATED_BODY()

public:
	virtual void BindViewModel(UViewModelBase* InViewModel) override;
	virtual void UnbindViewModel() override;

protected:
	virtual void NativeConstruct() override;

	// WBP_RoomInfo 슬롯 클래스 — 에디터에서 WBP_RoomInfo 지정
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<URoomInfoWidget> RoomInfoWidgetClass;

private:
	// Panel_Main 
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Create; // 방 생성하기 위한 패널 열기 (Panel_MakeRoom)

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_FindRoom; // 방 찾기

	// Panel_MakeRoom 
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> ET_RoomName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> ET_MaxPlayers;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> ET_Nickname;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Back; // 뒤로가기
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_CreateRoom; // 방 생성

	// Panel_RoomList
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> RoomListBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Join; // 방 선택 후 참가

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Back2; // 뒤로

	/*UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> txt_status;*/

	// 모드 선택 (optional)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCheckBox> cbx_mode_rift;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCheckBox> cbx_mode_dodgeball;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCheckBox> cbx_mode_event;

	// 버튼 핸들러
	UFUNCTION()
	void OnMakeRoomClicked();
	UFUNCTION()
	void OnFindRoomClicked();
	UFUNCTION()
	void OnConfirmCreate();
	UFUNCTION()
	void OnJoinClicked();
	UFUNCTION()
	void OnBackClicked();

	// 체크박스 핸들러
	UFUNCTION()
	void OnCbxRiftChanged(bool bIsChecked);
	UFUNCTION()
	void OnCbxDodgeballChanged(bool bIsChecked);
	UFUNCTION()
	void OnCbxEventChanged(bool bIsChecked);

	// ViewModel 콜백
	void OnSessionInfoReceived(const FLoLSessionInfo& Info);
	void OnFindDone(bool bSuccess);

	// 현재 선택된 슬롯 (라디오 그룹 관리용)
	TWeakObjectPtr<URoomInfoWidget> CurrentSelectedSlot;
	int32 SelectedSessionIndex = -1;

	void SwitchToPanel(int32 Index);
	void SelectMode(EMatchMode Mode);
};
