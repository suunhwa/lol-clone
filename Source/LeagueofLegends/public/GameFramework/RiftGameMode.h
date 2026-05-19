#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Type/RiftTypes.h"
#include "RiftGameMode.generated.h"

class ARiftPlayerState;
class ALoLPlayerStart;

UCLASS()
class LEAGUEOFLEGENDS_API ARiftGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARiftGameMode();

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	void OnNexusDestroyed(ETeam DestroyedTeam);
	// void OnChampionKilled(ARiftPlayerState* Killer, ARiftPlayerState* Victim);

	// 챔피언 처치 시 진입점. Assisters: 킬러 제외 어시스트 인정 아군 목록
	void OnChampionKilled(ARiftPlayerState* Killer, ARiftPlayerState* Victim, const TArray<ARiftPlayerState*>& Assisters = {});

	// 유닛(미니언/구조물) 처치 시 진입점. 미니언·타워 코드에서만 호출
	// UnitRowName: DataTable Row Name ("Minion_Melee", "Tower_Outer" 등)
	void OnUnitKilled(FName UnitRowName, FVector KillLocation, ETeam KillerTeam, AActor* DamageInstigator = nullptr);
	// KillerLevel 기준으로 ChampionKill XP 레벨 보정 계산
	static float CalcChampionKillXP(float BaseXP, int32 KillerLevel, int32 VictimLevel);
	// 게임 시간(분) 기준으로 유닛 XP 계산 (MaxXP 상한 적용)
	static float CalcUnitXP(const struct FUnitRewardExpRow& Row, float GameMinutes);
	// 레벨 + 게임 시간 기반 최종 부활 시간 계산 (반올림된 초 단위 반환)
	int32 CalculateRespawnTime(int32 ChampionLevel, float GameTimeSeconds, float AdditionalRespawnTime = 0.f) const;
	// Team 소속 아군 중 Location 반경 Radius 이내 PlayerState 수집
	TArray<ARiftPlayerState*> FindNearbyAllies(FVector Location, float Radius, ETeam Team) const;

	// 해당 팀의 부활 포인트 반환 (없으면 nullptr)
	class ALoLChampionRespawnPoint* FindRespawnPoint(ETeam Team) const;
	
private:
	void TryStartGame();
	void StartGame();
	void EndGame(ETeam WinningTeam);

	// BeginPlay에서 맵의 ALoLPlayerStart를 팀별로 수집
	void CollectSpawnPoints();
	// 팀 내 빈 슬롯을 PlayerState에 배정
	void AssignTeamSlot(ARiftPlayerState* PS);

	UPROPERTY(EditDefaultsOnly, Category = "Game")
	int32 PlayersPerTeam = 1;

	// SlotIndex 순 정렬된 팀별 스폰 포인트
	UPROPERTY()
	TArray<TObjectPtr<ALoLPlayerStart>> BlueSpawns;

	UPROPERTY()
	TArray<TObjectPtr<ALoLPlayerStart>> RedSpawns;
};
