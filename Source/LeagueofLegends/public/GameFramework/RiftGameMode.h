#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/RiftTypes.h"
#include "RiftGameMode.generated.h"

class ARiftPlayerState;

UCLASS()
class LEAGUEOFLEGENDS_API ARiftGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARiftGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	void OnNexusDestroyed(ETeam DestroyedTeam);
	void OnChampionKilled(ARiftPlayerState* Killer, ARiftPlayerState* Victim);

private:
	void AssignTeam(ARiftPlayerState* PS);
	void TryStartGame();
	void StartGame();
	void EndGame(ETeam WinningTeam);

	int32 BlueCount = 0;
	int32 RedCount = 0;

	static constexpr int32 PlayersPerTeam = 5;
};
