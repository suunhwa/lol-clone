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
		SlotData.DisplayName = Data->ChampionID.ToString();
		SlotData.Portrait = Data->PortraitTexture;
		CachedChampions.Add(SlotData);
	}

	OnChampionListReady.Broadcast(CachedChampions);
}

void UPickWindowViewModel::RefreshFromGameState()
{
	if (!GameState) { return; }

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
