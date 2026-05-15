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
	if (!GameState || !LocalPlayerState) { return false; }
	if (GameState->PlayerArray.IsEmpty()) { return false; }
	return GameState->PlayerArray[0] == LocalPlayerState;
}
