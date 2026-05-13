#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Type/RiftTypes.h"
#include "PickWindowGameMode.generated.h"

UCLASS()
class LEAGUEOFLEGENDS_API APickWindowGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	APickWindowGameMode();

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

	bool TrySwitchTeam(APlayerController* PC, ETeam NewTeam);
	void TryStartGame(APlayerController* PC);

private:
	int32 CountTeam(ETeam Team) const;
	ETeam GetTeamWithFewerPlayers() const;
	bool IsHost(APlayerController* PC) const;
	bool AllPlayersReady() const;

	static constexpr int32 MaxPerTeam = 5;
};
