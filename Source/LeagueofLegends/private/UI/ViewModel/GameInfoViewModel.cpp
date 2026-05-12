#include "UI/ViewModel/GameInfoViewModel.h"

#include "GameFramework/RiftPlayerState.h"
#include "GameFramework/RiftGameState.h"
#include "Type/RiftTypes.h"
#include "GameFramework/PlayerController.h"

void UGameInfoViewModel::Setup(ARiftPlayerState* InPS, ARiftGameState* InGS)
{
	PlayerState = InPS;
	GameState = InGS;
}

int32 UGameInfoViewModel::GetKills() const { return PlayerState ? PlayerState->GetKills() : 0; }
int32 UGameInfoViewModel::GetDeaths() const { return PlayerState ? PlayerState->GetDeaths() : 0; }
int32 UGameInfoViewModel::GetAssists() const { return PlayerState ? PlayerState->GetAssists() : 0; }
int32 UGameInfoViewModel::GetCS() const { return PlayerState ? PlayerState->GetCS() : 0; }

int32 UGameInfoViewModel::GetBlueKills() const
{
	return GameState ? GameState->GetTeamKills(ETeam::Blue) : 0;
}

int32 UGameInfoViewModel::GetRedKills() const
{
	return GameState ? GameState->GetTeamKills(ETeam::Red) : 0;
}

int32 UGameInfoViewModel::GetElapsedSeconds() const
{
	return GameState ? GameState->GetElapsedSeconds() : 0;
}

float UGameInfoViewModel::GetPingMs(APlayerController* PC) const
{
	return (PC && PC->PlayerState) ? PC->PlayerState->GetPingInMilliseconds() : 0.f;
}
