// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Type/RiftTypes.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class LEAGUEOFLEGENDS_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ALobbyGameMode();
	
	virtual void BeginPlay();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	// 팀 변경 
	bool TrySwitchTeam(APlayerController* PC, ETeam NewTeam);
	void TryStartChampionSelect(APlayerController* PC);
	void TryStartGame(APlayerController* PC);
	
private:
	int32 CountTeam(ETeam Team) const;
	
	ETeam GetTeamWithFewerPlayers() const;
	
	bool IsHost(APlayerController* PC) const;
	bool CanProceed() const;
	bool AllPlayersReady() const;

	// 팀 당 최소 인원: 1, 최대 인원: 5
	static constexpr int32 MaxPerTeam = 5;
	static constexpr int32 MinPerTeam = 1;
};



