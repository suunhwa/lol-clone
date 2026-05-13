// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "LoLSessionSubsystem.generated.h"



USTRUCT(BlueprintType)
struct FLoLSessionInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString RoomName;

	UPROPERTY(BlueprintReadOnly)
	FString HostName;

	UPROPERTY(BlueprintReadOnly)
	FString MaxPlayer;

	UPROPERTY(BlueprintReadOnly)
	int32 PingSpeed;

	UPROPERTY(BlueprintReadOnly)
	int32 Index;

	inline FString ToString()
	{
		return FString::Printf(TEXT("[%d] %s : %s  - %s, %dms"),
							   Index,
							   *RoomName,
							   *HostName,
							   *MaxPlayer,
							   PingSpeed);
	}
};


// 세션 생성 결과
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCreateSessionResult, bool, bWasSuccessful);

// 세션 검색 결과 (세션 하나 찾을 때마다 브로드캐스트)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionFound, const FLoLSessionInfo&, SessionInfo);

// 세션 검색 완료 (다 끝났을 때)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFindSessionsDone, bool, bWasSuccessful);

// 세션 참가 결과
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJoinSessionResult, bool, bWasSuccessful);

UCLASS()
class LEAGUEOFLEGENDS_API ULoLSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
public:
	IOnlineSessionPtr SessionInterface;

	UFUNCTION(BlueprintCallable)
	void CreateSession(FString RoomName, int32 MaxPlayer);
	
	// LobbyUI에 있는 start game 버튼으로 방 없으면 생성, 있으면 join
	UFUNCTION(BlueprintCallable)
	void FindOrCreateSession(FString InNickname, int32 MaxPlayers = 10);
	
	UFUNCTION(BlueprintCallable)
	void ExitRoom_BP() { ExitRoom(); }

	// session(host) 이름
	FString MySessionName = "LoL";

	UFUNCTION()
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	void GameToStart();

public:
	// 방 검색
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	void FindOtherSessions();

	void OnFindSessionsComplete(bool bWasSuccessful);

	// 세션 생성 결과 → OnCreateSessionComplete에서 브로드캐스트
	UPROPERTY(BlueprintAssignable)
	FOnCreateSessionResult OnCreateSessionResult;

	// 세션 하나 찾을 때마다 → OnFindSessionsComplete 루프 안에서 브로드캐스트
	UPROPERTY(BlueprintAssignable)
	FOnSessionFound OnSessionFound;

	// 검색 전체 완료 → OnFindSessionsComplete 끝에서 브로드캐스트
	UPROPERTY(BlueprintAssignable)
	FOnFindSessionsDone OnFindSessionsDone;

	// 참가 결과 → OnJoinSessionComplete에서 브로드캐스트
	UPROPERTY(BlueprintAssignable)
	FOnJoinSessionResult OnJoinSessionResult;

	// 세션(방) 입장
	void JoinSelectedSession(int32 Index);

	// 세션 입장 콜백
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	void ExitRoom();

	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	void OnNetworkFailure(UWorld* World,
						  UNetDriver* NetDriver,
						  ENetworkFailure::Type FailureType,
						  const FString& ErrorString = TEXT(""));
	
	bool IsInRoom();
	
	// issue: steam 한글 깨짐
	// 문자열을 uint8 배열로 만든 후 ASCII 코드로 변환
	FString StringBase64Encode(const FString& Str);
	FString StringBase64Decode(const FString& Str);
	
private:
	bool bFindOrCreateMode = false;
	FString PendingNickname;
	int32 PendingMaxPlayers = 10;

};
