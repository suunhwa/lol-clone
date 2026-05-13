// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/LobbyHUD.h"

#include "GameFramework/LoLGameInstance.h"
#include "GameFramework/LoLSessionSubsystem.h"
#include "UI/View/LobbyUIWidget.h"
#include "UI/ViewModel/LobbyUIViewModel.h"
#include "UI/ViewModel/SessionViewModel.h"
#include "Blueprint/UserWidget.h"
#include "UI/View/SessionWidget.h"


void ALobbyHUD::BeginPlay()
{
	Super::BeginPlay();

	if (!LobbyUIClass) { return; }

	APlayerController* PC = GetOwningPlayerController();
	if (!PC) { return; }

	auto* GI = GetGameInstance<ULoLGameInstance>();
	auto* Session = GI ? GI->GetSubsystem<ULoLSessionSubsystem>() : nullptr;

	// LobbyUI ViewModel
	LobbyUIVM = NewObject<ULobbyUIViewModel>(this);
	LobbyUIVM->Initialize();

	// Session ViewModel
	SessionVM = NewObject<USessionViewModel>(this);
	SessionVM->Setup(GI, Session);
	SessionVM->Initialize();

	// LobbyUI View 생성 + ViewModel 주입
	LobbyUIWidget = CreateWidget<ULobbyUIWidget>(PC, LobbyUIClass);
	if (LobbyUIWidget)
	{
		LobbyUIWidget->BindViewModel(LobbyUIVM);
		LobbyUIWidget->AddToViewport();
	}

	// WBP_Session은 WBP_LobbyUI 안에 embed되어 있으므로
	// LobbyUIWidget을 통해 SessionVM을 전달
	if (LobbyUIWidget)
		LobbyUIWidget->BindSessionWidget(SessionVM);
}
