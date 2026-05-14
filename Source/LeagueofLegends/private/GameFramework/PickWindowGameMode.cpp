#include "GameFramework/PickWindowGameMode.h"

#include "GameFramework/PickWindowHUD.h"
#include "GameFramework/RiftGameState.h"
#include "GameFramework/RiftPlayerController.h"
#include "GameFramework/RiftPlayerState.h"

APickWindowGameMode::APickWindowGameMode()
{
	bUseSeamlessTravel = true;
	GameStateClass = ARiftGameState::StaticClass();
	PlayerStateClass = ARiftPlayerState::StaticClass();
	PlayerControllerClass = ARiftPlayerController::StaticClass();
	HUDClass = APickWindowHUD::StaticClass();
}

void APickWindowGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (auto* GS = GetGameState<ARiftGameState>())
	{
		GS->SetPhase(EGamePhase::ChampionSelect);
	}
}

void APickWindowGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	auto* PS = NewPlayer->GetPlayerState<ARiftPlayerState>();
	if (!PS) { return; }

	PS->SetTeam(GetTeamWithFewerPlayers());
}

bool APickWindowGameMode::TrySwitchTeam(APlayerController* PC, ETeam NewTeam)
{
	auto* PS = PC->GetPlayerState<ARiftPlayerState>();
	if (!PS) { return false; }
	
	if (PS->GetTeam() == NewTeam)
	{
		return false;
	}
	
	if (CountTeam(NewTeam) >= MaxPerTeam)
	{
		return false;
	}

	PS->SetTeam(NewTeam);
	
	return true;
}

void APickWindowGameMode::TryStartGame(APlayerController* PC)
{
	if (!IsHost(PC)) { return; }

	// 호스트가 Start 누르면 자동으로 Ready 처리
	if (auto* PS = PC->GetPlayerState<ARiftPlayerState>())
	{
		PS->SetReady(true);
	}

	if (!AllPlayersReady()) { return; }

	GetWorld()->ServerTravel(TEXT("/Game/Maps/Lv_SummonerRift?listen"));
}

int32 APickWindowGameMode::CountTeam(ETeam Team) const
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

ETeam APickWindowGameMode::GetTeamWithFewerPlayers() const
{
	return (CountTeam(ETeam::Blue) <= CountTeam(ETeam::Red))
		       ? ETeam::Blue
		       : ETeam::Red;
}

bool APickWindowGameMode::IsHost(APlayerController* PC) const
{
	if (GameState->PlayerArray.IsEmpty()) { return false; }
	
	return GameState->PlayerArray[0] == PC->PlayerState;
}

bool APickWindowGameMode::AllPlayersReady() const
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
