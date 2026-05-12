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
	if (!PS) { return; }

	const FString TeamStr = PS->GetTeam() == ETeam::Blue ? TEXT("Blue") : TEXT("Red");
	PRINTLOG_SH(TEXT("Player joined. Team: %s, Blue: %d, Red: %d"), *TeamStr, BlueCount, RedCount);
	
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
	ARiftPlayerState* ps = Player ? Player->GetPlayerState<ARiftPlayerState>() : nullptr;
	if (!ps) return Super::ChoosePlayerStart_Implementation(Player);

	if (ps->GetTeam() == ETeam::None)
	{
		AssignTeam(ps);
	}

	FName TargetTag = (ps->GetTeam() == ETeam::Blue) ? FName("BlueTeam") : FName("RedTeam");

	TArray<APlayerStart*> ValidStarts;
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		if (It->PlayerStartTag == TargetTag)
			ValidStarts.Add(*It);
	}

	if (ValidStarts.IsEmpty()) return Super::ChoosePlayerStart_Implementation(Player);

	return ValidStarts[FMath::RandRange(0, ValidStarts.Num() - 1)];
}

void ARiftGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 테스트: 플레이어 수 상관없이 즉시 게임 시작 (실제 서비스 시 제거)
	StartGame();
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

void ARiftGameMode::AssignTeam(ARiftPlayerState* PS)
{
	if (BlueCount <= RedCount)
	{
		PS->SetTeam(ETeam::Blue);
		BlueCount++;
	}
	else
	{
		PS->SetTeam(ETeam::Red);
		RedCount++;
	}
}

void ARiftGameMode::TryStartGame()
{
	if (BlueCount >= PlayersPerTeam && RedCount >= PlayersPerTeam)
		StartGame();
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
