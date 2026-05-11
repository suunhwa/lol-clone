#include "Characters/Object/LoLInhibitor.h"

#include "LeagueofLegends.h"
#include "Characters/Minion/MinionSpawner.h"
#include "Components/ObjectStatComponent.h"
#include "Components/TagComponent.h"
#include "Kismet/GameplayStatics.h"

ALoLInhibitor::ALoLInhibitor()
{
	ObjectID = 11101;
}

void ALoLInhibitor::OnDestroyed()
{
	Super::OnDestroyed();

	// 1. 시각적/물리적 비활성화
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	// 2. 해당 팀 스포너에게 슈퍼 미니언 소환 명령
	UpdateSpawner(true);

	// 3. 리스폰 타이머 (데이터 테이블의 MechData.Respawn_Time 활용)
	float RespawnTime = MechData.Respawn_Time;
	if (RespawnTime <= 0.f) RespawnTime = 300.f; // 기본 5분

	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &ALoLInhibitor::Respawn, RespawnTime, false);
    
	PRINTLOG_HJ(TEXT("[억제기] 파괴됨! %s팀의 슈퍼 미니언 소환 시작."), (GetTeam() == ETeam::Red) ? TEXT("Blue") : TEXT("Red"));
}

void ALoLInhibitor::Respawn()
{
	bIsDestroyed = false;
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	// 1. 스탯 재초기화
	ObjectStatComp->InitObjectStats(StatData, RewardData, MechData);

	// 2. 다시 일반 미니언 소환으로 복구
	UpdateSpawner(false);

	PRINTLOG_HJ(TEXT("[억제기] 부활 완료! 일반 미니언 생성으로 복구됩니다."));
}

void ALoLInhibitor::UpdateSpawner(bool bSpawnSuper)
{
	// 1. 내 팀 정보 가져오기 (TagComponent 사용)
	UTagComponent* MyTag = FindComponentByClass<UTagComponent>();
	if (!MyTag) return;
    
	ETeam MyTeam = MyTag->GetTeam();

	// 2. 월드의 모든 스포너 검색
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMinionSpawner::StaticClass(), FoundActors);

	for (AActor* SpawnerActor : FoundActors)
	{
		AMinionSpawner* Spawner = Cast<AMinionSpawner>(SpawnerActor);
		if (Spawner)
		{
			// 3. 스포너의 TagComponent에서 팀 정보 가져오기
			UTagComponent* SpawnerTag = Spawner->FindComponentByClass<UTagComponent>();
			if (SpawnerTag)
			{
				// 상대 팀 스포너라면 슈퍼 미니언 모드 ON/OFF
				if (SpawnerTag->GetTeam() != MyTeam)
				{
					Spawner->bIsInhibitorDestroyed = bSpawnSuper;

					PRINTLOG_HJ(TEXT("[억제기 연동] 상대 팀(%s) 스포너 발견! 슈퍼 미니언 모드: %s"), 
						(SpawnerTag->GetTeam() == ETeam::Red) ? TEXT("Red") : TEXT("Blue"),
						bSpawnSuper ? TEXT("ON") : TEXT("OFF"));
				}
			}
		}
	}
}