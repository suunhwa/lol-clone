#include "GameFramework/RiftGameState.h"

#include "EngineUtils.h"
#include "Characters/LoLCharacterBase.h"
#include "FOW/FOWManager.h"
#include "GameFramework/RiftPlayerState.h"
#include "Net/UnrealNetwork.h"

ARiftGameState::ARiftGameState()
{
}

void ARiftGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ARiftGameState, CurrentPhase);
	DOREPLIFETIME(ARiftGameState, WinningTeam);
	DOREPLIFETIME(ARiftGameState, ElapsedSeconds);
	DOREPLIFETIME(ARiftGameState, BlueKills);
	DOREPLIFETIME(ARiftGameState, RedKills);
	DOREPLIFETIME(ARiftGameState, BlueTowers);
	DOREPLIFETIME(ARiftGameState, RedTowers);
	DOREPLIFETIME(ARiftGameState, BlueGold);
	DOREPLIFETIME(ARiftGameState, RedGold);
	DOREPLIFETIME(ARiftGameState, bBlueNexusAlive);
	DOREPLIFETIME(ARiftGameState, bRedNexusAlive);
	DOREPLIFETIME(ARiftGameState, FOWManager);
}

void ARiftGameState::SetPhase(EGamePhase InPhase)
{
	CurrentPhase = InPhase;
	HandlePhaseChanged();
}

void ARiftGameState::SetWinningTeam(ETeam InTeam)
{
	WinningTeam = InTeam;
}

void ARiftGameState::StartGameTimer()
{
	GetWorldTimerManager().SetTimer(GameTimerHandle, this, &ARiftGameState::IncrementTimer, 1.0f, true);
}

void ARiftGameState::StopGameTimer()
{
	GetWorldTimerManager().ClearTimer(GameTimerHandle);
}

void ARiftGameState::AddTeamKill(ETeam Team)
{
	if (Team == ETeam::Blue) BlueKills++;
	else if (Team == ETeam::Red) RedKills++;
}

void ARiftGameState::AddTeamGold(ETeam Team, int32 Amount)
{
	if (Team == ETeam::Blue) BlueGold += Amount;
	else if (Team == ETeam::Red) RedGold += Amount;
}

void ARiftGameState::AddTeamTower(ETeam Team)
{
	if (Team == ETeam::Blue) BlueTowers++;
	else if (Team == ETeam::Red) RedTowers++;
}

void ARiftGameState::SetNexusDestroyed(ETeam Team)
{
	if (Team == ETeam::Blue) bBlueNexusAlive = false;
	else if (Team == ETeam::Red) bRedNexusAlive = false;
}

int32 ARiftGameState::GetTeamKills(ETeam Team) const
{
	if (Team == ETeam::Blue) return BlueKills;
	if (Team == ETeam::Red) return RedKills;
	return 0;
}

int32 ARiftGameState::GetTeamGold(ETeam Team) const
{
	if (Team == ETeam::Blue) return BlueGold;
	if (Team == ETeam::Red) return RedGold;
	return 0;
}

void ARiftGameState::SetFOWManager(AFOWManager* Manager)
{
	FOWManager = Manager;
	
	// SightProvider 등록
	for (TActorIterator<ALoLCharacterBase> It(GetWorld()); It; ++It)
	{
		FOWManager->RegisterSightProvider(*It);
	}
	
	// FOWManager가 늦게 들어왔을 때, 이미 PlayerState에 Team이 있다면 즉시 푸시
	if (APlayerController* LocalPC = GetWorld()->GetFirstPlayerController())
	{
		if (ARiftPlayerState* PS = LocalPC->GetPlayerState<ARiftPlayerState>())
		{
			if (PS->GetTeam() != ETeam::None)
			{
				const ERiftSightTag Tag = (PS->GetTeam() == ETeam::Blue) 
					? ERiftSightTag::Blue : ERiftSightTag::Red;
				Manager->SetLocalClientTeam(Tag);
			}
		}
	}
}

void ARiftGameState::OnRep_CurrentPhase()
{
	HandlePhaseChanged();
	
	// TODO: 클라이언트 UI 페이즈 전환 처리
}

void ARiftGameState::HandlePhaseChanged()
{
	
}

void ARiftGameState::IncrementTimer()
{
	ElapsedSeconds++;
}

void ARiftGameState::OnRep_FOWManager()
{
	// FOWManager가 복제된 시점에 월드의 모든 캐릭터에게 등록 기회 부여
	for (TActorIterator<ALoLCharacterBase> It(GetWorld()); It; ++It)
	{
		FOWManager->RegisterSightProvider(*It);
	}
	
	// OnRep_Team이 먼저 왔을 때를 대비: FOWManager 복제 시점에 팀 재푸시
	if (APlayerController* LocalPC = GetWorld()->GetFirstPlayerController())
	{
		if (ARiftPlayerState* PS = LocalPC->GetPlayerState<ARiftPlayerState>())
		{
			if (PS->GetTeam() != ETeam::None)
			{
				const ERiftSightTag Tag = (PS->GetTeam() == ETeam::Blue)
					? ERiftSightTag::Blue : ERiftSightTag::Red;
				FOWManager->SetLocalClientTeam(Tag);
			}
		}
	}
}
