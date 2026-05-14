// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/LobbyHUD.h"

#include "LeagueofLegends.h"
#include "GameFramework/LoLGameInstance.h"
#include "GameFramework/LoLSessionSubsystem.h"
#include "UI/View/LobbyUIWidget.h"
#include "UI/ViewModel/LobbyUIViewModel.h"
#include "UI/ViewModel/SessionViewModel.h"
#include "Blueprint/UserWidget.h"
#include "UI/View/SessionWidget.h"
#include "UObject/ConstructorHelpers.h"


ALobbyHUD::ALobbyHUD()
{
	/*static ConstructorHelpers::FClassFinder<ULobbyUIWidget> LobbyUIBP(
		TEXT("/Game/UI/WBP_LobbyUI")
	);

	if (LobbyUIBP.Succeeded())
	{
		LobbyUIClass = LobbyUIBP.Class;
	}*/
}

void ALobbyHUD::BeginPlay()
{
	Super::BeginPlay();
	
	
	PRINTLOG_SH(TEXT("LobbyHUD BeginPlay"));

	if (!LobbyUIClass)
	{
		PRINTLOG_SH(TEXT("LobbyUIClass is NULL — BP_LobbyHUD에서 WBP_LobbyUI 할당했는지 확인"));
		return;
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		PRINTLOG_SH(TEXT("OwningPlayerController is NULL"));
		return;
	}

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
