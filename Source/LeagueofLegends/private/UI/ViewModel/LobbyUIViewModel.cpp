#include "UI/ViewModel/LobbyUIViewModel.h"

void ULobbyUIViewModel::Initialize()
{
	SelectedMode = EMatchMode::SummonersRift;
}

void ULobbyUIViewModel::SetMode(EMatchMode InMode)
{
	SelectedMode = InMode;
	OnModeChanged.Broadcast(SelectedMode);
}
