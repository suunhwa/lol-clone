#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModel/ViewModelBase.h"
#include "ChampionPortraitViewModel.generated.h"

class ALoLChampion;
class ARiftPlayerState;
class UTexture2D;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPortraitLevelUpdated, int32);

UCLASS()
class LEAGUEOFLEGENDS_API UChampionPortraitViewModel : public UViewModelBase
{
	GENERATED_BODY()

public:
	virtual void Initialize() override
	{
	}

	void Setup(ALoLChampion* InChampion);

	int32 GetLevel() const;
	UTexture2D* GetPortraitTexture() const;
	float GetXPProgress() const;       // 0~1, NativeTick에서 폴링
	bool IsChampionDead() const;       // 사망 오버레이 표시 여부
	float GetRespawnTimeRemaining() const; // 남은 부활 시간 (초)

	FOnPortraitLevelUpdated OnLevelUpdated;

private:
	UFUNCTION()
	void HandleLevelChanged(int32 NewLevel);

	UPROPERTY()
	TObjectPtr<ALoLChampion> Champion;

	UPROPERTY()
	TObjectPtr<ARiftPlayerState> PlayerState;
};
