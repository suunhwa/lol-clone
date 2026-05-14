#include "GameFramework/LobbyGameMode.h"

#include "GameFramework/LobbyHUD.h"

ALobbyGameMode::ALobbyGameMode()
{
	HUDClass = ALobbyHUD::StaticClass();
}
