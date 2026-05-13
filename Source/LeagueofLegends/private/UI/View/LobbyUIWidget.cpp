#include "UI/View/LobbyUIWidget.h"

#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Type/RiftTypes.h"
#include "UI/View/SessionWidget.h"
#include "UI/ViewModel/LobbyUIViewModel.h"
#include "UI/ViewModel/SessionViewModel.h"

void ULobbyUIWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (btn_exitgame)
	{
		btn_exitgame->OnClicked.AddDynamic(this, &ULobbyUIWidget::OnExitClicked);
	}
}

void ULobbyUIWidget::BindViewModel(UViewModelBase* InViewModel)
{
	Super::BindViewModel(InViewModel);
}

void ULobbyUIWidget::UnbindViewModel()
{
	Super::UnbindViewModel();
}

void ULobbyUIWidget::BindSessionWidget(USessionViewModel* InVM)
{
	if (WBP_Session)
	{
		WBP_Session->BindViewModel(InVM);
	}
}

void ULobbyUIWidget::OnLolClicked()
{
	if (auto* VM = Cast<ULobbyUIViewModel>(OwnerViewModel))
	{
		VM->SetMode(EMatchMode::SummonersRift);
	}
}

void ULobbyUIWidget::OnExitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
