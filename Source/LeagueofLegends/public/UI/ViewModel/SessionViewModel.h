#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModel/ViewModelBase.h"
#include "Type/RiftTypes.h"
#include "SessionViewModel.generated.h"

class ULoLGameInstance;
class ULoLSessionSubsystem;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSessionStatusChanged, bool /*bSuccess*/, const FString& /*Message*/);

UCLASS()
class LEAGUEOFLEGENDS_API USessionViewModel : public UViewModelBase
{
	GENERATED_BODY()

public:
	virtual void Initialize() override;
	virtual void Reset() override;

	void Setup(ULoLGameInstance* InGI, ULoLSessionSubsystem* InSession);

	void SetSelectedMode(EMatchMode InMode);
	void RequestFindOrCreate(const FString& Nickname, int32 MaxPlayers = 10);
	// Start 버튼용 — 검색 없이 즉시 방 생성 (레이스 컨디션 방지)
	void RequestCreate(const FString& Nickname, int32 MaxPlayers = 10);

	FOnSessionStatusChanged OnSessionStatusChanged;

private:
	UFUNCTION()
	void HandleCreateResult(bool bWasSuccessful);

	UFUNCTION()
	void HandleJoinResult(bool bWasSuccessful);

	UPROPERTY()
	TObjectPtr<ULoLGameInstance> GameInstance;

	UPROPERTY()
	TObjectPtr<ULoLSessionSubsystem> SessionSubsystem;
};
