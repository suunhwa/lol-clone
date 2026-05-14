#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModel/ViewModelBase.h"
#include "Type/RiftTypes.h"
#include "PickWindowViewModel.generated.h"

class ARiftGameState;
class ARiftPlayerState;
class UChampionDataSubsystem;

USTRUCT()
struct FPlayerSlotViewData
{
	GENERATED_BODY()

	UPROPERTY()
	FString Nickname;
	
	UPROPERTY()
	ETeam Team = ETeam::None;
	
	UPROPERTY()
	bool bIsReady = false;
	
	UPROPERTY()
	FName ChampionID = NAME_None;
};

USTRUCT()
struct FChampSlotViewData
{
	GENERATED_BODY()

	UPROPERTY()
	FName ChampionID;
	
	UPROPERTY()
	FString DisplayName;
	
	UPROPERTY()
	TObjectPtr<UTexture2D> Portrait;
};

DECLARE_MULTICAST_DELEGATE(FOnPickWindowUpdated);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnChampionListReady, const TArray<FChampSlotViewData>&);

UCLASS()
class LEAGUEOFLEGENDS_API UPickWindowViewModel : public UViewModelBase
{
	GENERATED_BODY()

public:
	virtual void Initialize() override;

	void Setup(ARiftGameState* InGS, ARiftPlayerState* InLocalPS, UChampionDataSubsystem* InChampSubsys);
	
	void RefreshFromGameState();

	TArray<FPlayerSlotViewData> GetBlueTeamPlayers() const;
	TArray<FPlayerSlotViewData> GetRedTeamPlayers() const;
	bool IsLocalPlayerHost() const;

	FOnPickWindowUpdated OnPickWindowUpdated;
	FOnChampionListReady OnChampionListReady;

private:
	UPROPERTY()
	TObjectPtr<ARiftGameState> GameState;
	
	UPROPERTY()
	TObjectPtr<ARiftPlayerState> LocalPlayerState;
	
	UPROPERTY()
	TObjectPtr<UChampionDataSubsystem> ChampSubsystem;

	TArray<FPlayerSlotViewData> CachedPlayers;
	TArray<FChampSlotViewData> CachedChampions;
};
