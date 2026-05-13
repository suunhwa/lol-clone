// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/LobbyGameMode.h"

#include "GameFramework/RiftGameState.h"
#include "GameFramework/RiftPlayerController.h"
#include "GameFramework/RiftPlayerState.h"

ALobbyGameMode::ALobbyGameMode()
{
	bUseSeamlessTravel = true;
	PlayerStateClass = ARiftPlayerState::StaticClass();
	PlayerControllerClass = ARiftPlayerController::StaticClass();
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	auto* PS = NewPlayer->GetPlayerState<ARiftPlayerState>();
	if (!PS) { return; }

	PS->SetTeam(GetTeamWithFewerPlayers());
}

bool ALobbyGameMode::TrySwitchTeam(APlayerController* PC, ETeam NewTeam)
{
	auto* PS = PC->GetPlayerState<ARiftPlayerState>();
	if (!PS) { return false; }
	if (PS->GetTeam() == NewTeam) { return false; }
	if (CountTeam(NewTeam) >= MaxPerTeam) { return false; }

	PS->SetTeam(NewTeam);
	return true;
}

void ALobbyGameMode::TryStartChampionSelect(APlayerController* PC)
{
	if (!IsHost(PC)) { return; }
	if (!CanProceed()) { return; }

	GetGameState<ARiftGameState>()->SetPhase(EGamePhase::ChampionSelect);
}

void ALobbyGameMode::TryStartGame(APlayerController* PC)
{
	if (!IsHost(PC)) { return; }
	if (!AllPlayersReady()) { return; }

	GetWorld()->ServerTravel(TEXT("/Game/Maps/Lv_SummonerRift?listen"));
}

int32 ALobbyGameMode::CountTeam(ETeam Team) const
{
	int32 Count = 0;

	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (auto* RPS = Cast<ARiftPlayerState>(PS))
		{
			if (RPS->GetTeam() == Team)
			{
				Count++;
			}
		}
	}
	
	return Count;
}

ETeam ALobbyGameMode::GetTeamWithFewerPlayers() const
{
	return (CountTeam(ETeam::Blue) <= CountTeam(ETeam::Red))
		       ? ETeam::Blue
		       : ETeam::Red;
}

bool ALobbyGameMode::IsHost(APlayerController* PC) const
{
	if (GameState->PlayerArray.IsEmpty())
	{
		return false;
	}

	return GameState->PlayerArray[0] == PC->PlayerState;
}

bool ALobbyGameMode::CanProceed() const
{
	return CountTeam(ETeam::Blue) >= MinPerTeam
		&& CountTeam(ETeam::Red) >= MinPerTeam;
}

bool ALobbyGameMode::AllPlayersReady() const
{
	if (GameState->PlayerArray.IsEmpty()) { return false; }
	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (auto* RPS = Cast<ARiftPlayerState>(PS))
		{
			if (!RPS->GetIsReady())
			{
				return false;
			}
		}
	}

	return true;
}
