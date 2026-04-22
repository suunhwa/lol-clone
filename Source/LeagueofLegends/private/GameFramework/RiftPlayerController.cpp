#include "GameFramework/RiftPlayerController.h"
#include "GameFramework/RiftPlayerState.h"

ARiftPlayerController::ARiftPlayerController()
{
	
}

void ARiftPlayerController::Server_SelectSummonerSpells_Implementation(ESummonerSpell Spell1, ESummonerSpell Spell2)
{
	ARiftPlayerState* PS = GetPlayerState<ARiftPlayerState>();
	if (!PS) return;

	PS->SetSummonerSpells(Spell1, Spell2);
}

void ARiftPlayerController::Server_SelectLane_Implementation(ELane Lane)
{
	ARiftPlayerState* PS = GetPlayerState<ARiftPlayerState>();
	if (!PS) return;

	PS->SetLane(Lane);
}
