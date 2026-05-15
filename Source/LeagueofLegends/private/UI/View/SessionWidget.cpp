// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/View/SessionWidget.h"

#include "LeagueofLegends.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"
#include "Type/RiftTypes.h"
#include "UI/ViewModel/SessionViewModel.h"

void USessionWidget::BindViewModel(UViewModelBase* InViewModel)
{
	Super::BindViewModel(InViewModel);
	if (auto* VM = Cast<USessionViewModel>(InViewModel))
	{
		VM->OnSessionStatusChanged.AddUObject(this, &USessionWidget::OnSessionStatus);
	}
		
}

void USessionWidget::UnbindViewModel()
{
	if (auto* VM = Cast<USessionViewModel>(OwnerViewModel))
	{
		VM->OnSessionStatusChanged.RemoveAll(this);
	}
	
	Super::UnbindViewModel();
}

void USessionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (btn_start)
	{
		btn_start->OnClicked.AddDynamic(this, &USessionWidget::OnStartClicked);
	}

	// button
	if (btn_mode_rift)
	{
		btn_mode_rift->OnClicked.AddDynamic(this, &USessionWidget::OnModeRiftClicked);
	}
	
	if (btn_mode_dodgeball)
	{
		btn_mode_dodgeball->OnClicked.AddDynamic(this, &USessionWidget::OnModeDodgeballClicked);
	}
	
	if (btn_mode_event)
	{
		btn_mode_event->OnClicked.AddDynamic(this, &USessionWidget::OnModeEventClicked);
	}

	// checkbox
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
	
	// 기본 선택: 소환사의 협곡
	SelectMode(EMatchMode::SummonersRift);
}

void USessionWidget::OnStartClicked()
{
	PRINTLOG_SH(TEXT("[SessionWidget] Start 버튼 클릭됨"));

	auto* VM = Cast<USessionViewModel>(OwnerViewModel);
	if (!VM)
	{
		PRINTLOG_SH(TEXT("[SessionWidget] OwnerViewModel이 NULL"));
		return;
	}

	FString Nickname;
	if (ET_namespace)
	{
		Nickname = ET_namespace->GetText().ToString();
	}
		

	PRINTLOG_SH(TEXT("[SessionWidget] RequestFindOrCreate 호출, Nickname=%s"), *Nickname);
	VM->RequestFindOrCreate(Nickname);
}

void USessionWidget::OnSessionStatus(bool bSuccess, const FString& Message)
{
	if (txt_status)
	{
		txt_status->SetText(FText::FromString(Message));
	}
}

void USessionWidget::OnModeRiftClicked()
{
	SelectMode(EMatchMode::SummonersRift); 
}

void USessionWidget::OnModeDodgeballClicked()
{
	SelectMode(EMatchMode::DodgeBall); 
}

void USessionWidget::OnModeEventClicked()
{
	SelectMode(EMatchMode::EventMode); 
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

	// GameInstance에 저장
	if (auto* VM = Cast<USessionViewModel>(OwnerViewModel))
	{
		VM->SetSelectedMode(Mode);
	}
		
}
