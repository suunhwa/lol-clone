// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/LoLSessionSubsystem.h"

#include <string>

#include "LeagueofLegends.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Online/OnlineSessionNames.h"
#include "Misc/CoreDelegates.h"

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

	FCoreDelegates::OnPreExit.AddUObject(this, &ULoLSessionSubsystem::HandlePreExit);
}

void ULoLSessionSubsystem::Deinitialize()
{
	FCoreDelegates::OnPreExit.RemoveAll(this);

	Super::Deinitialize();
}

void ULoLSessionSubsystem::CreateSession(FString RoomName, int32 MaxPlayer)
{
	if (bIsOperationPending)
	{
		PRINTLOG_SH(TEXT("CreateSession: 이미 작업 중, 무시"));
		return;
	}
	bIsOperationPending = true;

	// 기존 세션이 남아있으면 먼저 정리 후 재생성
	if (SessionInterface->GetNamedSession(FName(*MySessionName)))
	{
		PRINTLOG_SH(TEXT("CreateSession: 기존 세션 발견 → 정리 후 재생성"));
		PendingNickname = RoomName;
		PendingMaxPlayers = MaxPlayer;
		bPendingCreateAfterDestroy = true;
		SessionInterface->DestroySession(FName(*MySessionName));
		return;
	}

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

	// 9. 고유 게임 식별자 — 내 세션만 검색되도록 필터용
	sessionSettings.Set(FName("GAME_ID"),
	                    FString("P1_LOL_V0.1"),
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
	if (bWasSuccessful)
	{
		UGameplayStatics::OpenLevel(GetWorld(), TEXT("/Game/Maps/Lv_PickWindow"), true, TEXT("listen?port=7777"));
	}
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
	PendingNickname = InNickname;
	PendingMaxPlayers = MaxPlayers;

	// 로컬에 기존 세션 남아있으면 정리 후 재시도
	if (SessionInterface->GetNamedSession(FName(*MySessionName)))
	{
		PRINTLOG_SH(TEXT("FindOrCreateSession: 기존 세션 정리 후 재시도"));
		bPendingFindOrCreateAfterDestroy = true;
		SessionInterface->DestroySession(FName(*MySessionName));
		return;
	}

	/*// 리슨 서버는 세션 검색 없이 직접 생성 — FindOrCreate 쓰면 스테일 세션에 조인해버림
	if (GetWorld()->GetNetMode() == NM_ListenServer)
	{
		PRINTLOG_SH(TEXT("FindOrCreateSession: NM_ListenServer → CreateSession 직접 호출"));
		bFindOrCreateMode = false;
		CreateSession(InNickname, MaxPlayers);
		return;
	}*/

	bFindOrCreateMode = true;
	FindOtherSessions();
}

void ULoLSessionSubsystem::FindOtherSessions()
{
	// sharedptr 사용할 때 MakeShareable로 해줘야 함
	SessionSearch = MakeShareable(new FOnlineSessionSearch());

	// 세션 생성 시 bUsesPresence=true, bUseLobbiesIfAvailable=true 이므로 PRESENCE로 검색
	// SEARCH_LOBBIES 단독 사용 시 Steam 검색 실패 케이스 있음
	SessionSearch->QuerySettings.Set(FName(TEXT("PRESENCESEARCH")), true, EOnlineComparisonOp::Equals);

	// 2. LAN 여부
	SessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == FName("NULL");

	// 3. 최대 검색 세션 수
	SessionSearch->MaxSearchResults = 10;

	// 4. 세션 검색
	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void ULoLSessionSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	const auto& Results = SessionSearch->SearchResults;

	PRINTLOG_SH(TEXT("Find complete: Success=%d Results=%d"),
	            bWasSuccessful,
	            Results.Num());

	// 실패 + 결과 0개 → 진짜 실패 (네트워크/Steam 오류)
	if (!bWasSuccessful && Results.Num() == 0)
	{
		bIsOperationPending = false;
		bFindOrCreateMode = false;
		PRINTLOG_SH(TEXT("*** Session search failed and no results"));
		OnFindSessionsDone.Broadcast(false);
		return;
	}

	// 실패여도 결과가 있으면 일단 검사 (Steam OSS 특성 — 결과 있는데 false 반환 케이스)
	PRINTLOG_SH(TEXT("*** Search result count: %d"), Results.Num());

	if (bFindOrCreateMode)
	{
		bFindOrCreateMode = false;

		// 각 결과의 GAME_ID 출력 + 내 세션 필터링
		int32 MatchIndex = -1;
		for (int32 i = 0; i < Results.Num(); i++)
		{
			if (!Results[i].IsValid()) { continue; }

			FString GameID;
			Results[i].Session.SessionSettings.Get(FName("GAME_ID"), GameID);
			PRINTLOG_SH(TEXT("Result[%d] GameID=%s Owner=%s"),
			            i,
			            *GameID,
			            *Results[i].Session.OwningUserName);

			if (MatchIndex < 0 && GameID == TEXT("P1_LOL_V0.1"))
			{
				MatchIndex = i;
			}
		}

		if (MatchIndex >= 0)
		{
			PRINTLOG_SH(TEXT("FindOrCreate: 내 세션 발견(Slot%d), 자동 조인"), MatchIndex);
			bFindOrCreateFallback = true;
			JoinSelectedSession(MatchIndex);
		}
		else
		{
			PRINTLOG_SH(TEXT("FindOrCreate: 내 세션 없음 → 새 세션 생성"));
			bIsOperationPending = false;
			CreateSession(PendingNickname, PendingMaxPlayers);
		}

		return;
	}

	// 유효성 체크
	for (int i = 0; i < Results.Num(); i++)
	{
		auto sr = Results[i];
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

		// Join 실패 → 새 세션 생성 금지 (클라가 서버가 되는 문제 방지)
		if (bFindOrCreateFallback)
		{
			bFindOrCreateFallback = false;
			PRINTLOG_SH(TEXT("Join Failed → fallback 제거됨, 실패 처리"));
		}
	}

	bFindOrCreateFallback = false;
	bIsOperationPending = false;
	OnJoinSessionResult.Broadcast(Result == EOnJoinSessionCompleteResult::Success);
}

void ULoLSessionSubsystem::ExitRoom()
{
	SessionInterface->DestroySession(FName(*MySessionName));
}

void ULoLSessionSubsystem::QuitSession()
{
	/*if (GetWorld()->GetNetMode() == NM_ListenServer)
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
	*/

	if (!SessionInterface.IsValid())
	{
		return;
	}

	if (SessionInterface->GetNamedSession(FName(*MySessionName)))
	{
		PRINTLOG_SH(TEXT("QuitSession: DestroySession"));
		SessionInterface->DestroySession(FName(*MySessionName));
		return;
	}

	// 세션이 없으면 그냥 로비로 이동
	if (auto* PC = GetWorld()->GetFirstPlayerController())
	{
		PC->ClientTravel(TEXT("/Game/Maps/Lv_Lobby"), ETravelType::TRAVEL_Absolute);
	}
}

void ULoLSessionSubsystem::HandlePreExit()
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	if (SessionInterface->GetNamedSession(FName(*MySessionName)))
	{
		PRINTLOG_SH(TEXT("PreExit: DestroySession"));
		SessionInterface->DestroySession(FName(*MySessionName));
	}
}

void ULoLSessionSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	PRINTLOG_SH(TEXT("DestroySession Complete: %s, Success=%d"),
	            *SessionName.ToString(),
	            bWasSuccessful);

	// pending 플래그를 먼저 확인 — 리셋 전에 체크해야 진행 중인 작업 상태가 날아가지 않음
	if (bPendingFindOrCreateAfterDestroy)
	{
		bPendingFindOrCreateAfterDestroy = false;
		bIsOperationPending = false;
		PRINTLOG_SH(TEXT("OnDestroySessionComplete: 정리 완료 → FindOrCreate 재시도"));
		FindOrCreateSession(PendingNickname, PendingMaxPlayers);
		return;
	}

	if (bPendingJoinAfterDestroy)
	{
		bPendingJoinAfterDestroy = false;
		PRINTLOG_SH(TEXT("OnDestroySessionComplete: 정리 완료 → JoinSelectedSession(%d)"), PendingJoinIndex);
		JoinSelectedSession(PendingJoinIndex);
		return;
	}

	if (bPendingCreateAfterDestroy)
	{
		bPendingCreateAfterDestroy = false;
		bIsOperationPending = false;
		PRINTLOG_SH(TEXT("OnDestroySessionComplete: 정리 완료 → CreateSession 재시도"));
		CreateSession(PendingNickname, PendingMaxPlayers);
		return;
	}

	// pending 없음 → 플래그 전체 리셋 후 로비로
	bIsOperationPending = false;
	bFindOrCreateMode = false;
	bFindOrCreateFallback = false;

	if (auto* PC = GetWorld()->GetFirstPlayerController())
	{
		PC->ClientTravel(TEXT("/Game/Maps/Lv_Lobby"), ETravelType::TRAVEL_Absolute);
	}
	
	/*// 세션 정리 후 재생성 대기 중이면 맵 이동 없이 바로 생성
	if (bPendingCreateAfterDestroy)
	{
		bPendingCreateAfterDestroy = false;
		PRINTLOG_SH(TEXT("OnDestroySessionComplete: 재생성 → CreateSession"));
		CreateSession(PendingNickname, PendingMaxPlayers);
		return;
	}

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
	}*/
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
