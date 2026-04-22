#include "GameFramework/RiftGameState.h"
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
