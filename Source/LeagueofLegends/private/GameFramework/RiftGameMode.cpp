#include "GameFramework/RiftGameMode.h"
#include "Type/RiftTypes.h"
#include "GameFramework/RiftPlayerState.h"
#include "GameFramework/RiftGameState.h"
#include "GameFramework/RiftPlayerController.h"
#include "GameFramework/RiftHUD.h"
#include "GameFramework/PlayerStart.h"
#include "Characters/LoLPlayerStart.h"
#include "EngineUtils.h"
#include "LeagueofLegends.h"
#include "Characters/LoLChampion.h"
#include "Manager/ChampionDataSubsystem.h"
#include "Struct/ExpStruct.h"

ARiftGameMode::ARiftGameMode()
{
	PlayerStateClass = ARiftPlayerState::StaticClass();
	GameStateClass = ARiftGameState::StaticClass();
	PlayerControllerClass = ARiftPlayerController::StaticClass();
	HUDClass = ARiftHUD::StaticClass();
	DefaultPawnClass = ALoLChampion::StaticClass();
}

void ARiftGameMode::BeginPlay()
{
	Super::BeginPlay();
	CollectSpawnPoints();
}

void ARiftGameMode::CollectSpawnPoints()
{
	BlueSpawns.Reset();
	RedSpawns.Reset();

	for (TActorIterator<ALoLPlayerStart> It(GetWorld()); It; ++It)
	{
		if (It->Team == ETeam::Blue)     { BlueSpawns.Add(*It); }
		else if (It->Team == ETeam::Red) { RedSpawns.Add(*It); }
	}

	BlueSpawns.Sort([](const ALoLPlayerStart& A, const ALoLPlayerStart& B) { return A.SlotIndex < B.SlotIndex; });
	RedSpawns.Sort([](const ALoLPlayerStart& A, const ALoLPlayerStart& B)  { return A.SlotIndex < B.SlotIndex; });

	PRINTLOG_SH(TEXT("[SpawnPoints] Blue:%d Red:%d"), BlueSpawns.Num(), RedSpawns.Num());
}

void ARiftGameMode::AssignTeamSlot(ARiftPlayerState* PS)
{
	if (!PS || !GameState) { return; }

	TSet<int32> UsedSlots;
	for (APlayerState* OtherPS : GameState->PlayerArray)
	{
		if (auto* RPS = Cast<ARiftPlayerState>(OtherPS))
		{
			if (RPS != PS && RPS->GetTeam() == PS->GetTeam() && RPS->GetTeamSlotIndex() != INDEX_NONE)
			{
				UsedSlots.Add(RPS->GetTeamSlotIndex());
			}
		}
	}

	for (int32 i = 0; i < 5; ++i)
	{
		if (!UsedSlots.Contains(i))
		{
			PS->SetTeamSlotIndex(i);
			return;
		}
	}
}

void ARiftGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ARiftPlayerState* PS = NewPlayer->GetPlayerState<ARiftPlayerState>();
	if (!PS) { return; }

	if (PS->GetTeamSlotIndex() == INDEX_NONE)
	{
		AssignTeamSlot(PS);
	}

	PRINTLOG_SH(TEXT("[PostLogin] Team:%s Slot:%d"),
		PS->GetTeam() == ETeam::Blue ? TEXT("Blue") : TEXT("Red"),
		PS->GetTeamSlotIndex());

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
	if (!PS) { return Super::ChoosePlayerStart_Implementation(Player); }

	const bool bBlue = (PS->GetTeam() == ETeam::Blue);

	// Seamless Travel 시 PostLogin이 BeginPlay보다 먼저 호출될 수 있음 → 즉시 수집
	if (BlueSpawns.IsEmpty() && RedSpawns.IsEmpty())
	{
		PRINTLOG_SH(TEXT("[ChoosePlayerStart] 스폰 배열 비어있음 — 즉시 재수집"));
		CollectSpawnPoints();
	}

	TArray<TObjectPtr<ALoLPlayerStart>>& Spawns = bBlue ? BlueSpawns : RedSpawns;

	if (!Spawns.IsEmpty())
	{
		const int32 SlotIndex = PS->GetTeamSlotIndex();
		for (ALoLPlayerStart* S : Spawns)
		{
			if (S && S->SlotIndex == SlotIndex)
			{
				PRINTLOG_SH(TEXT("[ChoosePlayerStart] %s팀 Slot%d → %s"),
					bBlue ? TEXT("Blue") : TEXT("Red"), SlotIndex, *GetNameSafe(S));
				return S;
			}
		}
		return Spawns[0];
	}

	// ── ALoLPlayerStart 미배치 시 기존 태그 기반 폴백 ───────────────────────
	// if (PS->GetTeam() == ETeam::None)
	// {
	// 	int32 Blue = 0, Red = 0;
	// 	for (APlayerState* OtherPS : GameState->PlayerArray)
	// 		if (auto* RPS = Cast<ARiftPlayerState>(OtherPS))
	// 		{
	// 			if (RPS->GetTeam() == ETeam::Blue) Blue++;
	// 			else if (RPS->GetTeam() == ETeam::Red) Red++;
	// 		}
	// 	PS->SetTeam(Blue <= Red ? ETeam::Blue : ETeam::Red);
	// }
	// FName Tag = (PS->GetTeam() == ETeam::Blue) ? FName("BlueTeam") : FName("RedTeam");
	// TArray<APlayerStart*> ValidStarts;
	// for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	// {
	// 	if (It->PlayerStartTag == Tag) ValidStarts.Add(*It);
	// }
	// if (!ValidStarts.IsEmpty())
	// 	return ValidStarts[FMath::RandRange(0, ValidStarts.Num() - 1)];
	// ────────────────────────────────────────────────────────────────────────

	PRINTLOG_SH(TEXT("[ChoosePlayerStart] ALoLPlayerStart 없음 — 맵에 배치 필요"));
	return Super::ChoosePlayerStart_Implementation(Player);
}

void ARiftGameMode::OnNexusDestroyed(ETeam DestroyedTeam)
{
	ETeam Winner = (DestroyedTeam == ETeam::Blue) ? ETeam::Red : ETeam::Blue;
	EndGame(Winner);
}

void ARiftGameMode::OnChampionKilled(ARiftPlayerState* Killer,
	ARiftPlayerState* Victim,
	const TArray<ARiftPlayerState*>& Assisters)
{
	if (!Victim) { return; }
	Victim->AddDeath();

	if (!Killer || Killer == Victim) { return; }
	Killer->AddKill();
	
	for (ARiftPlayerState* A : Assisters)
	{
		if (A && A != Killer)
		{
			A->AddAssist();
		}
	}

	ARiftGameState* GS = GetGameState<ARiftGameState>();
	if (GS)
	{
		GS->AddTeamKill(Killer->GetTeam());
	}

	// XP 분배
	UChampionDataSubsystem* ExpSys = GetGameInstance()->GetSubsystem<UChampionDataSubsystem>();
	if (!ExpSys) { return; }

	const FChampionKillExpRow* Row = ExpSys->GetChampionKillExpRow(Victim->GetChampionLevel());
	if (!Row) { return; }

	// 킬러 + 어시스터를 하나의 수령자 목록으로 합산
	TArray<ARiftPlayerState*> Participants = {Killer};
	for (ARiftPlayerState* A : Assisters)
	{
		if (A && A != Killer)
		{
			Participants.Add(A);
		}
	}

	for (ARiftPlayerState* Participant : Participants)
	{
		if (!Participant) { continue; }
		// 레벨 보정 → 참여자 수로 균등 분배
		float XP = CalcChampionKillXP(Row->RewardXP, Participant->GetChampionLevel(), Victim->GetChampionLevel());
		Participant->AddXP(XP / Participants.Num());
	}
}

void ARiftGameMode::OnUnitKilled(FName UnitRowName, FVector KillLocation, ETeam KillerTeam)
{
	UChampionDataSubsystem* ExpSys = GetGameInstance()->GetSubsystem<UChampionDataSubsystem>();
	if (!ExpSys) { return; }

	const FUnitRewardExpRow* Row = ExpSys->GetUnitRewardRow(UnitRowName);
	if (!Row) { return; }

	ARiftGameState* GS = GetGameState<ARiftGameState>();
	float GameMinutes = GS ? GS->GetElapsedSeconds() / 60.0f : 0.0f;

	float UnitXP = CalcUnitXP(*Row, GameMinutes);
	TArray<ARiftPlayerState*> Nearby = FindNearbyAllies(KillLocation, Row->ExpRadius, KillerTeam);
	if (Nearby.IsEmpty()) { return; }

	// 단독이면 100%, 복수면 SharingMultiplier 보정 후 균등 분배
	float XPEach = Nearby.Num() == 1
		? UnitXP
		: UnitXP * Row->SharingMultiplier / Nearby.Num();

	for (ARiftPlayerState* PS : Nearby)
	{
		PS->AddXP(XPEach);
	}

}

float ARiftGameMode::CalcChampionKillXP(float BaseXP, int32 KillerLevel, int32 VictimLevel)
{
	int32 LevelDiff = VictimLevel - KillerLevel; // 양수: 적이 더 높음, 음수: 적이 더 낮음
	float Multiplier;
	if (LevelDiff > 0)
	{
		Multiplier = 1.0f + LevelDiff * 0.16f;
	}
	else if (LevelDiff < 0)
	{
		// 하한 20% 보장
		Multiplier = FMath::Max(0.2f, 1.0f + LevelDiff * 0.10f);
	}
	else
	{
		Multiplier = 1.0f;
	}
	
	return BaseXP * Multiplier;
}

float ARiftGameMode::CalcUnitXP(const struct FUnitRewardExpRow& Row, float GameMinutes)
{
	return FMath::Min(Row.BaseXP + Row.GrowthPerMinute * GameMinutes, Row.MaxXP);
}

TArray<ARiftPlayerState*> ARiftGameMode::FindNearbyAllies(FVector Location, float Radius, ETeam Team) const
{
	TArray<ARiftPlayerState*> Result;
	for (APlayerState* PS : GameState->PlayerArray)
	{
		ARiftPlayerState* RPS = Cast<ARiftPlayerState>(PS);
		if (!RPS || RPS->GetTeam() != Team) { continue; }

		APawn* Pawn = RPS->GetPawn();
		if (Pawn && FVector::Dist(Pawn->GetActorLocation(), Location) <= Radius)
		{
			Result.Add(RPS);
		}
	}
	return Result;
}

/*void ARiftGameMode::OnChampionKilled(ARiftPlayerState* Killer, ARiftPlayerState* Victim)
{
	if (!Victim) { return; }
	Victim->AddDeath();

	if (!Killer || Killer == Victim) { return; }
	Killer->AddKill();

	ARiftGameState* GS = GetGameState<ARiftGameState>();
	if (!GS) { return; }
	GS->AddTeamKill(Killer->GetTeam());
}*/

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

