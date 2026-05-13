#include "UI/ViewModel/SessionViewModel.h"

#include "GameFramework/LoLGameInstance.h"
#include "GameFramework/LoLSessionSubsystem.h"

void USessionViewModel::Setup(ULoLGameInstance* InGI, ULoLSessionSubsystem* InSession)
{
	GameInstance = InGI;
	SessionSubsystem = InSession;
}

void USessionViewModel::Initialize()
{
	if (!SessionSubsystem) { return; }

	SessionSubsystem->OnCreateSessionResult.AddDynamic(this, &USessionViewModel::HandleCreateResult);
	SessionSubsystem->OnJoinSessionResult.AddDynamic(this, &USessionViewModel::HandleJoinResult);
}

void USessionViewModel::Reset()
{
	if (!SessionSubsystem) { return; }

	SessionSubsystem->OnCreateSessionResult.RemoveDynamic(this, &USessionViewModel::HandleCreateResult);
	SessionSubsystem->OnJoinSessionResult.RemoveDynamic(this, &USessionViewModel::HandleJoinResult);
}

void USessionViewModel::SetSelectedMode(EMatchMode InMode)
{
	if (GameInstance)
	{
		GameInstance->SelectedMode = InMode;
	}
}

void USessionViewModel::RequestFindOrCreate(const FString& Nickname, int32 MaxPlayers)
{
	if (!GameInstance || !SessionSubsystem) { return; }

	GameInstance->Nickname = Nickname.IsEmpty() ? TEXT("Player") : Nickname;
	SessionSubsystem->FindOrCreateSession(GameInstance->Nickname, MaxPlayers);

	OnSessionStatusChanged.Broadcast(true, TEXT("Searching..."));
}

void USessionViewModel::HandleCreateResult(bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		OnSessionStatusChanged.Broadcast(false, TEXT("Failed to create session."));
	}
}

void USessionViewModel::HandleJoinResult(bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		OnSessionStatusChanged.Broadcast(false, TEXT("Failed to join session."));
	}
}
