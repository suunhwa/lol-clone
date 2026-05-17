#include "UI/ViewModel/SessionViewModel.h"

#include "LeagueofLegends.h"
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
	SessionSubsystem->OnSessionFound.AddDynamic(this, &USessionViewModel::HandleSessionFound);
	SessionSubsystem->OnFindSessionsDone.AddDynamic(this, &USessionViewModel::HandleFindSessionsDone);
}

void USessionViewModel::Reset()
{
	if (!SessionSubsystem) { return; }

	SessionSubsystem->OnCreateSessionResult.RemoveDynamic(this, &USessionViewModel::HandleCreateResult);
	SessionSubsystem->OnJoinSessionResult.RemoveDynamic(this, &USessionViewModel::HandleJoinResult);
	SessionSubsystem->OnSessionFound.RemoveDynamic(this, &USessionViewModel::HandleSessionFound);
	SessionSubsystem->OnFindSessionsDone.RemoveDynamic(this, &USessionViewModel::HandleFindSessionsDone);
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
	if (!GameInstance)
	{
		PRINTLOG_SH(TEXT("[SessionVM] GameInstance NULL"));
		return;
	}
	if (!SessionSubsystem)
	{
		PRINTLOG_SH(TEXT("[SessionVM] SessionSubsystem NULL"));
		return;
	}

	GameInstance->Nickname = Nickname.IsEmpty() ? TEXT("Player") : Nickname;
	
	PRINTLOG_SH(TEXT("[SessionVM] FindOrCreateSession 호출"));
	
	// SessionSubsystem->FindOrCreateSession(GameInstance->Nickname, MaxPlayers);

	OnSessionStatusChanged.Broadcast(true, TEXT("Searching..."));
}

void USessionViewModel::RequestCreate(const FString& RoomName, const FString& Nickname, int32 MaxPlayers)
{
	if (!GameInstance || !SessionSubsystem) { return; }

	GameInstance->Nickname = Nickname.IsEmpty() ? TEXT("Player") : Nickname;

	const FString Room = RoomName.IsEmpty() ? TEXT("My Room") : RoomName;
	PRINTLOG_SH(TEXT("[SessionVM] CreateSession — Room:%s Players:%d"), *Room, MaxPlayers);
	SessionSubsystem->CreateSession(Room, MaxPlayers);

	OnSessionStatusChanged.Broadcast(true, TEXT("Creating room..."));
}

void USessionViewModel::RequestFind()
{
	if (!SessionSubsystem) { return; }
	PRINTLOG_SH(TEXT("[SessionVM] FindOtherSessions"));
	SessionSubsystem->FindOtherSessions();
	OnSessionStatusChanged.Broadcast(true, TEXT("Searching..."));
}

void USessionViewModel::RequestJoin(int32 Index)
{
	if (!SessionSubsystem) { return; }
	PRINTLOG_SH(TEXT("[SessionVM] JoinSelectedSession(%d)"), Index);
	SessionSubsystem->JoinSelectedSession(Index);
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

void USessionViewModel::HandleSessionFound(const FLoLSessionInfo& Info)
{
	OnSessionInfoReceived.Broadcast(Info);
}

void USessionViewModel::HandleFindSessionsDone(bool bWasSuccessful)
{
	OnFindDone.Broadcast(bWasSuccessful);
}
