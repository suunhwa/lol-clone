#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Type/RiftTypes.h"
#include "RiftPlayerState.generated.h"

UCLASS()
class LEAGUEOFLEGENDS_API ARiftPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ARiftPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 서버 전용 setter
	void SetTeam(ETeam InTeam);
	void SetLane(ELane InLane);
	void SetSummonerSpells(ESummonerSpell Spell1, ESummonerSpell Spell2);
	void SetDisconnected(bool bDisconnected);
	void AddKill();
	void AddDeath();
	void AddAssist();
	void AddCS(int32 Amount = 1);
	void AddGold(int32 Amount);
	void AddXP(float Amount);
	
	void SetSelectedChampion(FName ChampionID);
	void SetReady(bool bReady);

	FName GetSelectedChampion() const { return SelectedChampionID; }
	bool GetIsReady() const { return bIsReady; }

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerLevelUp, int32 /*NewLevel*/);
	FOnPlayerLevelUp OnLevelUp;

	ETeam GetTeam() const { return Team; }
	ELane GetLane() const { return Lane; }
	int32 GetKills() const { return Kills; }
	int32 GetDeaths() const { return Deaths; }
	int32 GetAssists() const { return Assists; }
	int32 GetCS() const { return CS; }
	int32 GetGold() const { return Gold; }
	int32 GetTotalGold() const { return TotalGold; }
	int32 GetChampionLevel() const { return ChampionLevel; }
	float GetXP() const { return XP; }
	bool IsDisconnected() const { return bIsDisconnected; }

private:
	UPROPERTY(ReplicatedUsing = OnRep_Team)
	ETeam Team = ETeam::None;

	UPROPERTY(Replicated)
	ELane Lane = ELane::None;

	UPROPERTY(Replicated)
	ESummonerSpell SummonerSpell1 = ESummonerSpell::None;

	UPROPERTY(Replicated)
	ESummonerSpell SummonerSpell2 = ESummonerSpell::None;

	UPROPERTY(Replicated)
	int32 Kills = 0;

	UPROPERTY(Replicated)
	int32 Deaths = 0;

	UPROPERTY(Replicated)
	int32 Assists = 0;

	UPROPERTY(Replicated)
	int32 CS = 0;

	UPROPERTY(Replicated)
	int32 Gold = 0;

	UPROPERTY(Replicated)
	int32 TotalGold = 0;

	UPROPERTY(Replicated)
	int32 ChampionLevel = 1;

	UPROPERTY(Replicated)
	float XP = 0.0f;

	UPROPERTY(Replicated)
	bool bIsDisconnected = false;
	
	UPROPERTY(ReplicatedUsing = OnRep_IsReady)
	bool bIsReady = false;

	UPROPERTY(Replicated)
	FName SelectedChampionID = NAME_None;

	UFUNCTION()
	void OnRep_IsReady();

	UFUNCTION()
	void OnRep_Team();
};
