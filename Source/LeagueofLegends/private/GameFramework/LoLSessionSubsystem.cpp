// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/LoLSessionSubsystem.h"

#include <string>

#include "LeagueofLegends.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Online/OnlineSessionNames.h"

void ULoLSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (auto subsys = IOnlineSubsystem::Get())
	{
		// subsystem으로부터 session interface 가져옴
		SessionInterface = subsys->GetSessionInterface();

		SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this,
		                                                              &ULoLSessionSubsystem::OnCreateSessionComplete);
		SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this,
		                                                             &ULoLSessionSubsystem::OnFindSessionsComplete);
		SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &ULoLSessionSubsystem::OnJoinSessionComplete);
		SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this,
		                                                               &ULoLSessionSubsystem::OnDestroySessionComplete);
	}

	if (GEngine)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &ULoLSessionSubsystem::OnNetworkFailure);
	}
}

void ULoLSessionSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void ULoLSessionSubsystem::CreateSession(FString RoomName, int32 MaxPlayer)
{
	// session 설정 변수
	FOnlineSessionSettings sessionSettings;

	// 1. Dedicated server 접속 여부
	sessionSettings.bIsDedicated = false;

	// 2. 랜선(local) 매칭을 할지 Steam 매칭을 사용할지 여부
	FName SubsysName = IOnlineSubsystem::Get()->GetSubsystemName();
	sessionSettings.bIsLANMatch = SubsysName == "NULL";

	// 3. 매칭이 온라인을 통해 노출될지 여부
	// false이면 초대를 통해서만 입장 가능
	sessionSettings.bShouldAdvertise = true;

	// 4. 온라인 상태 (presence) 정보를 활용할지 여부
	sessionSettings.bUsesPresence = true;
	sessionSettings.bUseLobbiesIfAvailable = true;

	// 5. 게임 진행 중 참여 허가할지 여부
	sessionSettings.bAllowJoinViaPresence = false;
	sessionSettings.bAllowJoinInProgress = false;

	// 6. session에 참여할 수 있는 공개 (public) 연결의 최대 허용 수
	sessionSettings.NumPublicConnections = MaxPlayer;

	// 7. custom room name 설정
	sessionSettings.Set(FName("ROOM_NAME"),
	                    StringBase64Encode(RoomName),
	                    EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	// 8. host name 설정
	sessionSettings.Set(FName("HOST_NAME"),
	                    StringBase64Encode(MySessionName),
	                    EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	// netID
	FUniqueNetIdPtr netID = GetWorld()->GetFirstLocalPlayerFromController()->GetUniqueNetIdForPlatformUser().
	                                    GetUniqueNetId();

	PRINTLOG_SH(TEXT("Create Session Start : %s"), *MySessionName);
	SessionInterface->CreateSession(*netID, FName(MySessionName), sessionSettings);
}

void ULoLSessionSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	bIsOperationPending = false;
	PRINTLOG_SH(TEXT("Session Name : %s, bWasSuccessful : %d"), *MySessionName, bWasSuccessful);
	// 세션 생성 후 Lv_Lobby에서 대기 — 클라이언트가 찾아서 접속하면 서버가 PickWindow로 이동
	OnCreateSessionResult.Broadcast(bWasSuccessful);
}

void ULoLSessionSubsystem::FindOrCreateSession(FString InNickname, int32 MaxPlayers)
{
	if (bIsOperationPending)
	{
		PRINTLOG_SH(TEXT("FindOrCreateSession: 이미 작업 중, 무시"));
		return;
	}

	bIsOperationPending = true;
	bFindOrCreateMode = true;
	PendingNickname = InNickname;
	PendingMaxPlayers = MaxPlayers;
	FindOtherSessions();
}

void ULoLSessionSubsystem::FindOtherSessions()
{
	// sharedptr 사용할 때 MakeShareable로 해줘야 함
	SessionSearch = MakeShareable(new FOnlineSessionSearch());

	// 1. 세션 검색 조건 설정
	SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

	// 2. LAN 여부
	SessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == FName("NULL");

	// 3. 최대 검색 세션 수
	SessionSearch->MaxSearchResults = 10;

	// 4. 세션 검색
	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void ULoLSessionSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	// 찾기 실패 시
	if (bWasSuccessful == false)
	{
		bIsOperationPending = false;
		PRINTLOG_SH(TEXT("*** Session search failed"));
		OnFindSessionsDone.Broadcast(false);
		return;
	}

	// session 검색 결과 배열
	auto results = SessionSearch->SearchResults;
	PRINTLOG_SH(TEXT("*** Search result count: %d"), results.Num());

	if (bFindOrCreateMode)
	{
		bFindOrCreateMode = false;
		if (results.Num() > 0 && results[0].IsValid())
		{
			PRINTLOG_SH(TEXT("FindOrCreate: session found, auto-joining"));
			JoinSelectedSession(0);
		}
		else
		{
			PRINTLOG_SH(TEXT("FindOrCreate: session not found"));
			CreateSession(PendingNickname, PendingMaxPlayers);
		}
		
		return;
	}
	
	// 유효성 체크
	// for (auto sr : results)
	for (int i = 0; i < results.Num(); i++)
	{
		auto sr = results[i];
		if (sr.IsValid() == false)
		{
			continue;
		}

		// session info 구조체 선언
		FLoLSessionInfo SessionInfo;
		SessionInfo.Index = i;

		FString RoomName;
		FString HostName;

		/*FString roomName;
		sr.Session.SessionSettings.Get(FName("ROOM_NAME"), roomName);*/
		sr.Session.SessionSettings.Get(FName("ROOM_NAME"), RoomName);

		/*FString hostName;
		sr.Session.SessionSettings.Get(FName("HOST_NAME"), hostName);*/
		sr.Session.SessionSettings.Get(FName("HOST_NAME"), HostName);
		SessionInfo.RoomName = StringBase64Decode(RoomName);
		SessionInfo.HostName = StringBase64Decode(HostName);

		// session host name
		FString UserName = sr.Session.OwningUserName;

		// 입장 가능한 플레이어 수
		int32 MaxPlayerCount = sr.Session.SessionSettings.NumPublicConnections;

		// 현재 입장한 플레이어 수
		// (최대 인원 수 - 현재 입장 가능한 수)
		int32 CurrentPlayerCount = MaxPlayerCount - sr.Session.NumOpenPublicConnections;
		// NumOpenPublicConnections : 남은 자리 수 (현재 입장 가능한 수)

		SessionInfo.MaxPlayer = FString::Printf(TEXT("%d/%d"), CurrentPlayerCount, MaxPlayerCount);

		// 핑 정보
		// int32 pingSpeed = sr.PingInMs;
		SessionInfo.PingSpeed = sr.PingInMs;

		PRINTLOG_SH(TEXT("*** %s"), *SessionInfo.ToString());

		// delegate로 위젯에 알려주기
		OnSessionFound.Broadcast(SessionInfo);
	}

	OnFindSessionsDone.Broadcast(true);
}

void ULoLSessionSubsystem::JoinSelectedSession(int32 Index)
{
	auto sr = SessionSearch->SearchResults[Index];
	SessionInterface->JoinSession(0, FName(MySessionName), sr);
}

void ULoLSessionSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		auto pc = GetWorld()->GetFirstPlayerController();
		FString url;
		SessionInterface->GetResolvedConnectString(SessionName, url);

		// NULL subsystem이 LAN 세션의 포트를 0으로 반환하는 경우 기본 포트로 교정
		if (url.EndsWith(TEXT(":0")))
		{
			url = url.LeftChop(2) + TEXT(":7777");
		}

		PRINTLOG_SH(TEXT("Join URL (fixed) : %s"), *url);

		if (!url.IsEmpty())
		{
			pc->ClientTravel(url, ETravelType::TRAVEL_Absolute);
		}
	}
	else
	{
		PRINTLOG_SH(TEXT("Join Session Failed : %d"), Result);
		// ETravelType::TRAVEL_Relative;
	}
	
	bIsOperationPending = false;
	OnJoinSessionResult.Broadcast(Result == EOnJoinSessionCompleteResult::Success);
}

void ULoLSessionSubsystem::ExitRoom()
{
	SessionInterface->DestroySession(FName(*MySessionName));
}

void ULoLSessionSubsystem::QuitSession()
{
	if (GetWorld()->GetNetMode() == NM_ListenServer)
	{
		// 호스트: 세션 파괴 → OnDestroySessionComplete에서 ServerTravel
		ExitRoom();
	}
	else
	{
		// 클라이언트: 세션 소유자가 아니므로 파괴 없이 바로 이동
		if (auto* PC = GetWorld()->GetFirstPlayerController())
		{
			PC->ClientTravel(TEXT("/Game/Maps/Lv_Lobby"), ETravelType::TRAVEL_Absolute);
		}
	}
}

void ULoLSessionSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (GetWorld()->GetNetMode() == NM_ListenServer)
	{
		// 호스트: ServerTravel → 모든 클라이언트 자동 연결 해제 후 함께 Lv_Lobby로
		GetWorld()->ServerTravel(TEXT("/Game/Maps/Lv_Lobby?listen"));
	}
	else
	{
		// 클라이언트: 로컬에서만 Lv_Lobby로 이동
		if (auto* PC = GetWorld()->GetFirstPlayerController())
		{
			PC->ClientTravel(TEXT("/Game/Maps/Lv_Lobby"), TRAVEL_Absolute);
		}
	}
}

void ULoLSessionSubsystem::OnNetworkFailure(UWorld* World,
                                            UNetDriver* NetDriver,
                                            ENetworkFailure::Type FailureType,
                                            const FString& ErrorString)
{
	if (FailureType == ENetworkFailure::Type::ConnectionLost)
	{
		ExitRoom();
	}
}

bool ULoLSessionSubsystem::IsInRoom()
{
	FUniqueNetIdPtr netID = GetWorld()->GetFirstLocalPlayerFromController()->GetUniqueNetIdForPlatformUser().
	                                    GetUniqueNetId();
	return SessionInterface->IsPlayerInSession(FName(*MySessionName), *netID);
}

FString ULoLSessionSubsystem::StringBase64Encode(const FString& Str)
{
	// Set 할 때 :: FString -> UTF8(std::string) -> TArray<uint8> -> base64로 encode
	std::string UTF8String = TCHAR_TO_UTF8(*Str);
	TArray<uint8> ArrayData(reinterpret_cast<const uint8*>(UTF8String.c_str()), UTF8String.size());
	return FBase64::Encode(ArrayData);
}

FString ULoLSessionSubsystem::StringBase64Decode(const FString& Str)
{
	// Get 할 때 :: base64 로 decode -> TArray<uint8> -> TCHAR
	TArray<uint8> ArrayData;
	FBase64::Decode(Str, ArrayData);
	std::string UTF8String(reinterpret_cast<char*>(ArrayData.GetData()), ArrayData.Num());
	return UTF8_TO_TCHAR(UTF8String.c_str());
}

