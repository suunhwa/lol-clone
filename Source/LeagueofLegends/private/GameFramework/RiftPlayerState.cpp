#include "GameFramework/RiftPlayerState.h"

#include "LeagueofLegends.h"
#include "Characters/LoLCharacterBase.h"
#include "Components/StatComponent.h"
#include "FOW/FOWManager.h"
#include "GameFramework/RiftGameState.h"
#include "GameFramework/RiftPlayerCameraManager.h"
#include "Manager/ChampionDataSubsystem.h"
#include "Net/UnrealNetwork.h"

void ARiftPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (bTeamDebug)
	{
		SetTeam(DebugTeam);
	}
}

ARiftPlayerState::ARiftPlayerState()
{
}

void ARiftPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	if (ARiftPlayerState* Dest = Cast<ARiftPlayerState>(PlayerState))
	{
		Dest->Team = Team;
		Dest->TeamSlotIndex = TeamSlotIndex;
		Dest->Lane = Lane;
		Dest->SummonerSpell1 = SummonerSpell1;
		Dest->SummonerSpell2 = SummonerSpell2;
		Dest->SelectedChampionID = SelectedChampionID;
	}
}

void ARiftPlayerState::OverrideWith(APlayerState* PlayerState)
{
	Super::OverrideWith(PlayerState);

	if (ARiftPlayerState* Src = Cast<ARiftPlayerState>(PlayerState))
	{
		Team = Src->Team;
		TeamSlotIndex = Src->TeamSlotIndex;
		Lane = Src->Lane;
		SummonerSpell1 = Src->SummonerSpell1;
		SummonerSpell2 = Src->SummonerSpell2;
		SelectedChampionID = Src->SelectedChampionID;
	}
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
	DOREPLIFETIME(ARiftPlayerState, TeamSlotIndex);
	DOREPLIFETIME(ARiftPlayerState, bIsDisconnected);
	DOREPLIFETIME(ARiftPlayerState, bIsReady);
	DOREPLIFETIME(ARiftPlayerState, SelectedChampionID);
	DOREPLIFETIME(ARiftPlayerState, bIsDead);
	DOREPLIFETIME(ARiftPlayerState, RespawnEndServerTime);
}

void ARiftPlayerState::SetTeamSlotIndex(int32 InIndex)
{
	TeamSlotIndex = InIndex;
}

void ARiftPlayerState::SetDeadState(bool bDead, float EndServerTime)
{
	bIsDead = bDead;
	RespawnEndServerTime = EndServerTime;
	// 리슨 서버 로컬 플레이어는 OnRep가 호출되지 않으므로 직접 호출
	OnRep_DeathState();
}

void ARiftPlayerState::OnRep_DeathState()
{
	// 이 PlayerState를 소유한 로컬 플레이어의 카메라에만 효과 적용
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC || !PC->IsLocalController()) { return; }

	ARiftPlayerCameraManager* Cam = Cast<ARiftPlayerCameraManager>(PC->PlayerCameraManager);
	if (Cam)
	{
		Cam->SetDeathDesaturation(bIsDead);
	}
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
	if (!HasAuthority() || ChampionLevel >= 18) { return; }

	UChampionDataSubsystem* ExpSys = GetGameInstance()->GetSubsystem<UChampionDataSubsystem>();
	if (!ExpSys) { return; }

	XP += Amount;
	OnXPChanged.Broadcast(XP, ChampionLevel);

	while (ChampionLevel < 18)
	{
		// 행 N+1 = 레벨 N에서 N+1로 가는 데 필요한 XP
		const FPlayerLevelExpRow* Row = ExpSys->GetPlayerLevelExpRow(ChampionLevel + 1);
		// if (!Row || XP < Row->RequiredXP) { break; }

		if (!Row)
		{
			break;
		}
		if (XP < Row->RequiredXP) { break; }

		XP -= Row->RequiredXP;
		ChampionLevel++;

		OnLevelUp.Broadcast(ChampionLevel);

		// StatComp 레벨 동기화 — UI와 스탯 성장이 여기서 읽음
		if (APawn* Pawn = GetPawn())
		{
			if (UStatComponent* StatComp = Pawn->FindComponentByClass<UStatComponent>())
			{
				StatComp->SetLevel(ChampionLevel);
			}
		}
	}
}

void ARiftPlayerState::SetSelectedChampion(FName ChampionID)
{
	SelectedChampionID = ChampionID;
}

void ARiftPlayerState::SetReady(bool bReady)
{
	bIsReady = bReady;
	OnReadyChanged.Broadcast(bReady);
}

void ARiftPlayerState::OnRep_IsReady()
{
	OnReadyChanged.Broadcast(bIsReady);
}

void ARiftPlayerState::OnRep_Team()
{
	// 팀이 복제되면 이 캐릭터의 HP바 색상을 재평가
	if (APawn* Pawn = GetPawn())
	{
		if (ALoLCharacterBase* Char = Cast<ALoLCharacterBase>(Pawn))
		{
			Char->RefreshHUDDisplay();
		}
	}
	
	// 진짜 로컬 플레이어의 PlayerState일 때만 FOWManager에 통보
	APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	if (!LocalPC || LocalPC->PlayerState != this) { return; }

	if (Team == ETeam::None) { return; }

	if (ARiftGameState* GS = GetWorld()->GetGameState<ARiftGameState>())
	{
		if (AFOWManager* FOW = GS->GetFOWManager())
		{
			const ERiftSightTag Tag = (Team == ETeam::Blue) ? ERiftSightTag::Blue : ERiftSightTag::Red;
			FOW->SetLocalClientTeam(Tag);
		}
	}
}

void ARiftPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();
	OnNameChanged.Broadcast(GetPlayerName());

	// 구독 타이밍이 맞지 않을 경우를 대비한 직접 갱신
	if (APawn* Pawn = GetPawn())
	{
		if (ALoLCharacterBase* Char = Cast<ALoLCharacterBase>(Pawn))
		{
			Char->RefreshHUDDisplay();
		}
	}
}

void ARiftPlayerState::OnRep_ChampionLevel()
{
	OnXPChanged.Broadcast(XP, ChampionLevel);
}

void ARiftPlayerState::OnRep_XP()
{
	OnXPChanged.Broadcast(XP, ChampionLevel);
}
