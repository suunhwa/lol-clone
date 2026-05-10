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

// 레벨별 필요 경험치 (레벨 1→2부터 17→18)
static const float GXPRequired[] =
{
	280, 380, 480, 580, 680, 780, 880,
	1080, 1280, 1480, 1780, 2080, 2380,
	2680, 2980, 3280, 3480
};

void ARiftPlayerState::AddXP(float Amount)
{
	if (ChampionLevel >= 18) return;

	XP += Amount;

	while (ChampionLevel < 18 && XP >= GXPRequired[ChampionLevel - 1])
	{
		XP -= GXPRequired[ChampionLevel - 1];
		ChampionLevel++;
		OnLevelUp.Broadcast(ChampionLevel);
	}
}

void ARiftPlayerState::OnRep_Team()
{
	// TODO
}
