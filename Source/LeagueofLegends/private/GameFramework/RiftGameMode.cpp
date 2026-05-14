#include "GameFramework/RiftGameMode.h"
#include "Type/RiftTypes.h"
#include "GameFramework/RiftPlayerState.h"
#include "GameFramework/RiftGameState.h"
#include "GameFramework/RiftPlayerController.h"
#include "GameFramework/RiftHUD.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "LeagueofLegends.h"
#include "Characters/LoLChampion.h"

ARiftGameMode::ARiftGameMode()
{
	PlayerStateClass = ARiftPlayerState::StaticClass();
	GameStateClass = ARiftGameState::StaticClass();
	PlayerControllerClass = ARiftPlayerController::StaticClass();
	HUDClass = ARiftHUD::StaticClass();
	DefaultPawnClass = ALoLChampion::StaticClass();
}

void ARiftGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ARiftPlayerState* PS = NewPlayer->GetPlayerState<ARiftPlayerState>();
	if (!PS) return;

	PRINTLOG_SH(TEXT("Player joined game. Team: %s"),
	            PS->GetTeam() == ETeam::Blue ? TEXT("Blue") : TEXT("Red"));

	TryStartGame();
}

void ARiftGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
}

void ARiftGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	ARiftPlayerState* ps = Exiting->GetPlayerState<ARiftPlayerState>();
	if (!ps) { return; }

	ps->SetDisconnected(true);
}

AActor* ARiftGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	ARiftPlayerState* PS = Player ? Player->GetPlayerState<ARiftPlayerState>() : nullptr;
	if (!PS)
	{
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	// 팀 None일 때 블루/레드 교대 배정
	if (PS->GetTeam() == ETeam::None)
	{
		int32 Blue = 0, Red = 0;
		for (APlayerState* OtherPS : GameState->PlayerArray)
			if (auto* RPS = Cast<ARiftPlayerState>(OtherPS))
			{
				if (RPS->GetTeam() == ETeam::Blue) Blue++;
				else if (RPS->GetTeam() == ETeam::Red) Red++;
			}
		PS->SetTeam(Blue <= Red ? ETeam::Blue : ETeam::Red);
	}

	FName Tag = (PS->GetTeam() == ETeam::Blue) ? FName("BlueTeam") : FName("RedTeam");

	TArray<APlayerStart*> ValidStarts;
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		if (It->PlayerStartTag == Tag)
		{
			ValidStarts.Add(*It);
		}
	}


	if (ValidStarts.IsEmpty())
	{
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	return ValidStarts[FMath::RandRange(0, ValidStarts.Num() - 1)];
}

void ARiftGameMode::OnNexusDestroyed(ETeam DestroyedTeam)
{
	ETeam Winner = (DestroyedTeam == ETeam::Blue) ? ETeam::Red : ETeam::Blue;
	EndGame(Winner);
}

void ARiftGameMode::OnChampionKilled(ARiftPlayerState* Killer, ARiftPlayerState* Victim)
{
	if (!Victim) { return; }
	Victim->AddDeath();

	if (!Killer || Killer == Victim) { return; }
	Killer->AddKill();

	ARiftGameState* GS = GetGameState<ARiftGameState>();
	if (!GS) { return; }
	GS->AddTeamKill(Killer->GetTeam());
}

void ARiftGameMode::TryStartGame()
{
	int32 Blue = 0, Red = 0;

	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (auto* RPS = Cast<ARiftPlayerState>(PS))
		{
			if (RPS->GetTeam() == ETeam::Blue)
			{
				Blue++;
			}
			else if (RPS->GetTeam() == ETeam::Red)
			{
				Red++;
			}
		}
	}

	if (Blue >= PlayersPerTeam && Red >= PlayersPerTeam)
	{
		StartGame();
	}
}

void ARiftGameMode::StartGame()
{
	ARiftGameState* gs = GetGameState<ARiftGameState>();
	if (!gs) { return; }

	gs->SetPhase(EGamePhase::InGame);
	gs->StartGameTimer();
}

void ARiftGameMode::EndGame(ETeam WinningTeam)
{
	ARiftGameState* gs = GetGameState<ARiftGameState>();
	if (!gs) { return; }

	gs->SetPhase(EGamePhase::GameOver);
	gs->SetWinningTeam(WinningTeam);
	gs->StopGameTimer();
}
