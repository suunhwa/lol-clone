#include "GameFramework/RiftPlayerState.h"
#include "Net/UnrealNetwork.h"

ARiftPlayerState::ARiftPlayerState()
{
}

void ARiftPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ARiftPlayerState, Team);
	DOREPLIFETIME(ARiftPlayerState, Lane);
	DOREPLIFETIME(ARiftPlayerState, SummonerSpell1);
	DOREPLIFETIME(ARiftPlayerState, SummonerSpell2);
	DOREPLIFETIME(ARiftPlayerState, Kills);
	DOREPLIFETIME(ARiftPlayerState, Deaths);
	DOREPLIFETIME(ARiftPlayerState, Assists);
	DOREPLIFETIME(ARiftPlayerState, CS);
	DOREPLIFETIME(ARiftPlayerState, Gold);
	DOREPLIFETIME(ARiftPlayerState, TotalGold);
	DOREPLIFETIME(ARiftPlayerState, ChampionLevel);
	DOREPLIFETIME(ARiftPlayerState, XP);
	DOREPLIFETIME(ARiftPlayerState, bIsDisconnected);
}

void ARiftPlayerState::SetTeam(ETeam InTeam)
{
	Team = InTeam;
	OnRep_Team();
}

void ARiftPlayerState::SetLane(ELane InLane)
{
	Lane = InLane;
}

void ARiftPlayerState::SetSummonerSpells(ESummonerSpell Spell1, ESummonerSpell Spell2)
{
	SummonerSpell1 = Spell1;
	SummonerSpell2 = Spell2;
}

void ARiftPlayerState::SetDisconnected(bool bDisconnected)
{
	bIsDisconnected = bDisconnected;
}

void ARiftPlayerState::AddKill()
{
	Kills++;
}

void ARiftPlayerState::AddDeath()
{
	Deaths++;
}

void ARiftPlayerState::AddAssist()
{
	Assists++;
}

void ARiftPlayerState::AddCS(int32 Amount)
{
	CS += Amount;
}

void ARiftPlayerState::AddGold(int32 Amount)
{
	Gold += Amount;
	TotalGold += Amount;
}

void ARiftPlayerState::AddXP(float Amount)
{
	XP += Amount;
	// TODO: 레벨업 조건 & 레벨업
}

void ARiftPlayerState::OnRep_Team()
{
	// TODO
}
