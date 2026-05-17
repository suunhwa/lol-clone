#include "UI/ViewModel/PickWindowViewModel.h"

#include "Characters/Data/ChampionData.h"
#include "GameFramework/RiftGameState.h"
#include "GameFramework/RiftPlayerState.h"
#include "Manager/ChampionDataSubsystem.h"

void UPickWindowViewModel::Setup(ARiftGameState* InGS,
                                 ARiftPlayerState* InLocalPS,
                                 UChampionDataSubsystem* InChampSubsys)
{
	GameState = InGS;
	LocalPlayerState = InLocalPS;
	ChampSubsystem = InChampSubsys;
}

void UPickWindowViewModel::Initialize()
{
	if (!ChampSubsystem) { return; }

	CachedChampions.Reset();
	for (UChampionData* Data : ChampSubsystem->GetAllChampions())
	{
		if (!Data) { continue; }

		FChampSlotViewData SlotData;
		SlotData.ChampionID = Data->ChampionID;
		SlotData.DisplayName = Data->ChampionName_KR.IsNone()
			? Data->ChampionID.ToString()
			: Data->ChampionName_KR.ToString();
		SlotData.Portrait = Data->PortraitTexture;
		CachedChampions.Add(SlotData);
	}

	OnChampionListReady.Broadcast(CachedChampions);
}

void UPickWindowViewModel::RefreshFromGameState()
{
	// Setup 시점에 GameState가 null이었으면 월드에서 다시 가져옴
	if (!GameState)
	{
		if (UWorld* World = GetWorld())
			GameState = World->GetGameState<ARiftGameState>();
		if (!GameState) { return; }
	}

	CachedPlayers.Reset();

	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (auto* RPS = Cast<ARiftPlayerState>(PS))
		{
			FPlayerSlotViewData Data;
			Data.Nickname = RPS->GetPlayerName();
			Data.Team = RPS->GetTeam();
			Data.bIsReady = RPS->GetIsReady();
			Data.ChampionID = RPS->GetSelectedChampion();
			Data.PlayerState = RPS;

			// 챔피언 선택됐으면 한글 이름 + 초상화 조회
			if (ChampSubsystem && Data.ChampionID != NAME_None)
			{
				if (UChampionData* ChampData = ChampSubsystem->GetChampionData(Data.ChampionID))
				{
					Data.ChampionName_KR = ChampData->ChampionName_KR;
					Data.ChampPortrait   = ChampData->PortraitTexture;
				}
			}

			CachedPlayers.Add(Data);

			// Ready 상태 변경 시 즉시 슬롯 갱신 (중복 구독 방지)
			if (!RPS->OnReadyChanged.IsBoundToObject(this))
			{
				RPS->OnReadyChanged.AddUObject(this, &UPickWindowViewModel::OnPlayerReadyChanged);
			}
		}
	}

	OnPickWindowUpdated.Broadcast();
}

TArray<FPlayerSlotViewData> UPickWindowViewModel::GetBlueTeamPlayers() const
{
	TArray<FPlayerSlotViewData> Result;
	for (const FPlayerSlotViewData& D : CachedPlayers)
	{
		if (D.Team == ETeam::Blue)
		{
			Result.Add(D);
		}
	}
	return Result;
}

TArray<FPlayerSlotViewData> UPickWindowViewModel::GetRedTeamPlayers() const
{
	TArray<FPlayerSlotViewData> Result;
	for (const FPlayerSlotViewData& D : CachedPlayers)
	{
		if (D.Team == ETeam::Red)
		{
			Result.Add(D);
		}
	}
	return Result;
}

bool UPickWindowViewModel::IsLocalPlayerHost() const
{
	if (!LocalPlayerState) { return false; }

	// PlayerArray[0] 순서는 복제 타이밍에 따라 불안정 →
	// 현재 월드의 NetMode로 판별 (ListenServer 머신 = 호스트)
	UWorld* World = GetWorld();
	if (!World) { return false; }

	const ENetMode NetMode = World->GetNetMode();
	return NetMode == NM_ListenServer || NetMode == NM_Standalone;
}

bool UPickWindowViewModel::IsLocalPlayerReady() const
{
	return LocalPlayerState ? LocalPlayerState->GetIsReady() : false;
}
