#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModel/ViewModelBase.h"
#include "Type/RiftTypes.h"
#include "SessionViewModel.generated.h"

class ULoLGameInstance;
class ULoLSessionSubsystem;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSessionStatusChanged, bool /*bSuccess*/, const FString& /*Message*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionInfoReceived, const FLoLSessionInfo& /*Info*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnFindDone, bool /*bSuccess*/);

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
	void RequestCreate(const FString& RoomName, const FString& Nickname, int32 MaxPlayers = 10);
	void RequestFind();
	void RequestJoin(int32 Index);

	FOnSessionStatusChanged OnSessionStatusChanged;
	FOnSessionInfoReceived  OnSessionInfoReceived;
	FOnFindDone             OnFindDone;

private:
	UFUNCTION()
	void HandleCreateResult(bool bWasSuccessful);

	UFUNCTION()
	void HandleJoinResult(bool bWasSuccessful);

	UFUNCTION()
	void HandleSessionFound(const FLoLSessionInfo& Info);

	UFUNCTION()
	void HandleFindSessionsDone(bool bWasSuccessful);

	UPROPERTY()
	TObjectPtr<ULoLGameInstance> GameInstance;

	UPROPERTY()
	TObjectPtr<ULoLSessionSubsystem> SessionSubsystem;
};
