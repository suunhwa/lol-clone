#include "GameFramework/LobbyGameMode.h"

#include "GameFramework/LobbyHUD.h"
#include "GameFramework/RiftPlayerController.h"
#include "GameFramework/RiftPlayerState.h"

ALobbyGameMode::ALobbyGameMode()
{
	bUseSeamlessTravel = true;
	HUDClass = ALobbyHUD::StaticClass();
	PlayerStateClass = ARiftPlayerState::StaticClass();
	PlayerControllerClass = ARiftPlayerController::StaticClass();
}
