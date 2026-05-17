// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/View/SessionWidget.h"

#include "LeagueofLegends.h"
#include "UI/View/RoomInfoWidget.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/WidgetSwitcher.h"
#include "Type/RiftTypes.h"
#include "UI/ViewModel/SessionViewModel.h"

void USessionWidget::BindViewModel(UViewModelBase* InViewModel)
{
	Super::BindViewModel(InViewModel);
	if (auto* VM = Cast<USessionViewModel>(InViewModel))
	{
		VM->OnSessionInfoReceived.AddUObject(this, &USessionWidget::OnSessionInfoReceived);
		VM->OnFindDone.AddUObject(this, &USessionWidget::OnFindDone);
	}
}

void USessionWidget::UnbindViewModel()
{
	if (auto* VM = Cast<USessionViewModel>(OwnerViewModel))
	{
		VM->OnSessionInfoReceived.RemoveAll(this);
		VM->OnFindDone.RemoveAll(this);
	}
	Super::UnbindViewModel();
}

void USessionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Create)
	{
		Btn_Create->OnClicked.AddDynamic(this, &USessionWidget::OnMakeRoomClicked);
	}
	if (Btn_FindRoom)
	{
		Btn_FindRoom->OnClicked.AddDynamic(this, &USessionWidget::OnFindRoomClicked);
	}
	if (Btn_Back)
	{
		Btn_Back->OnClicked.AddDynamic(this, &USessionWidget::OnBackClicked);
	}
	if (Btn_CreateRoom)
	{
		Btn_CreateRoom->OnClicked.AddDynamic(this, &USessionWidget::OnConfirmCreate);
	}
	if (Btn_Join)
	{
		Btn_Join->OnClicked.AddDynamic(this, &USessionWidget::OnJoinClicked);
	}
	if (Btn_Back2)
	{
		Btn_Back2->OnClicked.AddDynamic(this, &USessionWidget::OnBackClicked);
	}

	if (cbx_mode_rift)
	{
		cbx_mode_rift->OnCheckStateChanged.AddDynamic(this, &USessionWidget::OnCbxRiftChanged);
	}
	if (cbx_mode_dodgeball)
	{
		cbx_mode_dodgeball->OnCheckStateChanged.AddDynamic(this, &USessionWidget::OnCbxDodgeballChanged);
	}
	if (cbx_mode_event)
	{
		cbx_mode_event->OnCheckStateChanged.AddDynamic(this, &USessionWidget::OnCbxEventChanged);
	}

	SelectMode(EMatchMode::SummonersRift);
	SwitchToPanel(0);
}

// 패널 전환 
void USessionWidget::SwitchToPanel(int32 Index)
{
	if (WidgetSwitcher)
	{
		WidgetSwitcher->SetActiveWidgetIndex(Index);
	}
		
}

// Panel_Main
void USessionWidget::OnMakeRoomClicked()
{
	SwitchToPanel(1);
}

void USessionWidget::OnFindRoomClicked()
{
	SwitchToPanel(2);

	if (RoomListBox)
		RoomListBox->ClearChildren();

	CurrentSelectedSlot = nullptr;
	SelectedSessionIndex = -1;

	if (auto* VM = Cast<USessionViewModel>(OwnerViewModel))
	{
		VM->RequestFind();
	}
}

// Panel_MakeRoom
void USessionWidget::OnConfirmCreate()
{
	auto* VM = Cast<USessionViewModel>(OwnerViewModel);
	if (!VM) { return; }

	FString RoomName = ET_RoomName ? ET_RoomName->GetText().ToString() : TEXT("My Room");
	FString CountStr = ET_MaxPlayers ? ET_MaxPlayers->GetText().ToString() : TEXT("10");
	FString Nickname = ET_Nickname ? ET_Nickname->GetText().ToString() : TEXT("Player");

	const int32 MaxPlayers = FMath::Clamp(FCString::Atoi(*CountStr), 2, 10);

	VM->RequestCreate(RoomName, Nickname, MaxPlayers);
}

// Panel_RoomList
void USessionWidget::OnSessionInfoReceived(const FLoLSessionInfo& Info)
{
	if (!RoomInfoWidgetClass || !RoomListBox) { return; }

	URoomInfoWidget* RoomSlot = CreateWidget<URoomInfoWidget>(this, RoomInfoWidgetClass);
	if (!RoomSlot) { return; }

	RoomSlot->SetInfo(Info);

	RoomSlot->OnSelected.BindLambda([this](int32 Index, URoomInfoWidget* ClickedSlot)
	{
		// 이전 선택 해제
		if (CurrentSelectedSlot.IsValid())
			CurrentSelectedSlot->SetSelected(false);

		// 새 슬롯 선택
		ClickedSlot->SetSelected(true);
		CurrentSelectedSlot = ClickedSlot;
		SelectedSessionIndex = Index;
	});

	RoomListBox->AddChild(RoomSlot);

	if (SelectedSessionIndex < 0)
	{
		SelectedSessionIndex = Info.Index;
	}
		
}

void USessionWidget::OnFindDone(bool bSuccess)
{
	// txt_status 없음 — 필요하면 헤더에 추가
}

void USessionWidget::OnJoinClicked()
{
	if (SelectedSessionIndex < 0) { return; }

	auto* VM = Cast<USessionViewModel>(OwnerViewModel);
	if (!VM) { return; }

	FString Nickname = ET_Nickname ? ET_Nickname->GetText().ToString() : TEXT("Player");
	VM->RequestJoin(SelectedSessionIndex, Nickname);
}

void USessionWidget::OnBackClicked()
{
	SwitchToPanel(0);
}

// 상태 메시지 


// 모드 선택
void USessionWidget::SelectMode(EMatchMode Mode)
{
	if (cbx_mode_rift)
	{
		cbx_mode_rift->SetIsChecked(Mode == EMatchMode::SummonersRift);
	}
	
	if (cbx_mode_dodgeball)
	{
		cbx_mode_dodgeball->SetIsChecked(Mode == EMatchMode::DodgeBall);
	}
	
	if (cbx_mode_event)
	{
		cbx_mode_event->SetIsChecked(Mode == EMatchMode::EventMode);
	}

	if (auto* VM = Cast<USessionViewModel>(OwnerViewModel))
	{
		VM->SetSelectedMode(Mode);
	}
}

void USessionWidget::OnCbxRiftChanged(bool bIsChecked)
{
	if (bIsChecked)
	{
		SelectMode(EMatchMode::SummonersRift);
	}
}
void USessionWidget::OnCbxDodgeballChanged(bool bIsChecked)
{
	if (bIsChecked)
	{
		SelectMode(EMatchMode::DodgeBall);
	}
}
void USessionWidget::OnCbxEventChanged(bool bIsChecked)
{
	if (bIsChecked)
	{
		SelectMode(EMatchMode::EventMode);
	}
}
