#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Type/RiftTypes.h"
#include "RiftGameState.generated.h"

class AFOWManager;

// 구조물 상태 변화를 알리기 위한 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStructureStateChanged);

UCLASS()
class LEAGUEOFLEGENDS_API ARiftGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ARiftGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void SetPhase(EGamePhase InPhase);
	void SetWinningTeam(ETeam InTeam);
	void StartGameTimer();
	void StopGameTimer();
	void AddTeamKill(ETeam Team);
	void AddTeamGold(ETeam Team, int32 Amount);
	void AddTeamTower(ETeam Team);
	void SetNexusDestroyed(ETeam Team);

	EGamePhase GetPhase() const { return CurrentPhase; }
	ETeam GetWinningTeam() const { return WinningTeam; }
	int32 GetElapsedSeconds() const { return ElapsedSeconds; }
	int32 GetTeamKills(ETeam Team) const;
	int32 GetTeamGold(ETeam Team) const;
	
	UFUNCTION(BlueprintCallable)
	AFOWManager* GetFOWManager() const { return FOWManager; }
	UFUNCTION(BlueprintCallable)
	void SetFOWManager(AFOWManager* Manager) { FOWManager = Manager; }

	
	// 타워가 깨지거나 억제기가 재생성될 때 호출될 이벤트
	UPROPERTY(BlueprintAssignable, Category = "GameEvents")
	FOnStructureStateChanged OnStructureStateChanged;

	// 편의를 위해 이벤트 호출 함수 추가
	void BroadcastStructureStateChanged() { OnStructureStateChanged.Broadcast(); }
	
private:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentPhase)
	EGamePhase CurrentPhase = EGamePhase::Lobby;

	UPROPERTY(Replicated)
	ETeam WinningTeam = ETeam::None;

	UPROPERTY(Replicated)
	int32 ElapsedSeconds = 0;

	UPROPERTY(Replicated)
	int32 BlueKills = 0;

	UPROPERTY(Replicated)
	int32 RedKills = 0;

	UPROPERTY(Replicated)
	int32 BlueTowers = 0;

	UPROPERTY(Replicated)
	int32 RedTowers = 0;

	UPROPERTY(Replicated)
	int32 BlueGold = 0;

	UPROPERTY(Replicated)
	int32 RedGold = 0;

	UPROPERTY(Replicated)
	bool bBlueNexusAlive = true;

	UPROPERTY(Replicated)
	bool bRedNexusAlive = true;

	FTimerHandle GameTimerHandle;

	UFUNCTION()
	void OnRep_CurrentPhase();
	
	void HandlePhaseChanged();

	void IncrementTimer();
	
private:
	UPROPERTY()
	TObjectPtr<AFOWManager> FOWManager;
};
