#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModel/ViewModelBase.h"
#include "GameInfoViewModel.generated.h"

class ARiftPlayerState;
class ARiftGameState;

UCLASS()
class LEAGUEOFLEGENDS_API UGameInfoViewModel : public UViewModelBase
{
	GENERATED_BODY()

public:
	virtual void Initialize() override {}

	void Setup(ARiftPlayerState* InPS, ARiftGameState* InGS);

	int32 GetKills() const;
	int32 GetDeaths() const;
	int32 GetAssists() const;
	int32 GetCS() const;
	int32 GetBlueKills() const;
	int32 GetRedKills() const;
	int32 GetElapsedSeconds() const;
	float GetPingMs(APlayerController* PC) const;

private:
	UPROPERTY()
	TObjectPtr<ARiftPlayerState> PlayerState;

	UPROPERTY()
	TObjectPtr<ARiftGameState> GameState;
};
