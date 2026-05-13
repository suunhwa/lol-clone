#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModel/ViewModelBase.h"
#include "Type/RiftTypes.h"
#include "LobbyUIViewModel.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnModeChanged, EMatchMode);

UCLASS()
class LEAGUEOFLEGENDS_API ULobbyUIViewModel : public UViewModelBase
{
	GENERATED_BODY()

public:
	virtual void Initialize() override;

	void SetMode(EMatchMode InMode);
	EMatchMode GetSelectedMode() const { return SelectedMode; }

	FOnModeChanged OnModeChanged;

private:
	EMatchMode SelectedMode = EMatchMode::SummonersRift;
};
