// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/LoLMinion.h"
#include "Kismet/GameplayStatics.h"
#include "AStar/AStarGridManager.h"
#include "Characters/Nexus/Nexus.h"

ALoLMinion::ALoLMinion()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ALoLMinion::BeginPlay()
{
	Super::BeginPlay();
	// 0.5초마다 타겟 갱신
	GetWorldTimerManager().SetTimer(TargetUpdateTimerHandle,
		this, &ALoLMinion::UpdateTarget, 0.5f, true);
	// 월드에서 그리드 매니저 찾아오기
	GridManager = Cast<AAStarGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AAStarGridManager::StaticClass()));
}

void ALoLMinion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentPath.Num() > 0)
	{
		for (int32 i = 0; i < CurrentPath.Num() - 1; i++)
		{
			DrawDebugLine(GetWorld(), CurrentPath[i], CurrentPath[i+1], FColor::Yellow, false, -1.0f, 0, 5.0f);
		}
	}
	
	
	if (!TargetPlayer || !GridManager) return;
	
	// 경로가 없으면 무조건 찾기 시도
	if (CurrentPath.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s is Requesting Path to %s"), *GetName(), *TargetPlayer->GetName());
		CurrentPath = GridManager->FindPath(GetActorLocation(), TargetPlayer->GetActorLocation());
	}

	// 2. A* 경로가 있다면 경로 따라가기
	if (CurrentPath.Num() > 0)
	{
		FVector NextPoint = CurrentPath[0];
		FVector Direction = NextPoint - GetActorLocation();
		Direction.Z = 0.0f;

		if (Direction.Size() < 50.0f) 
		{
			CurrentPath.RemoveAt(0); // 지점 도달 시 다음 지점으로
		}
		else
		{
			Direction.Normalize();
			SetActorLocation(GetActorLocation() + (Direction * MoveSpeed * DeltaTime));
			SetActorRotation(Direction.Rotation());
		}
	}
	// 3. 경로가 없는데 타겟만 있다면 (혹은 A* 실패 시) 직선 이동 (Fallback)
	else if (TargetPlayer)
	{
		FVector Direction = TargetPlayer->GetActorLocation() - GetActorLocation();
		Direction.Z = 0.0f;
        
		Direction.Normalize();
		SetActorLocation(GetActorLocation() + (Direction * MoveSpeed * DeltaTime));
		SetActorRotation(Direction.Rotation());
		UE_LOG(LogTemp, Error, TEXT("AStar Failed to find path!"));
	}
}

void ALoLMinion::UpdateTarget()
{
	// 성능을 위해 주기적으로 근처의 액터를 검색
	TArray<AActor*> FoundActors;
	// 월드의 모든 캐릭터를 가져와 태그 비교
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("Character"), FoundActors);

	AActor* BestTarget = nullptr;
	float ClosestDistance = MAX_FLT;
	
	// 내 팀 태그 있는지 확인
	FName MyTeamTag = (Tags.Num() > 0) ? Tags[0] : NAME_None;
	
	// 1순위 : 주변 적 유닛 (챔피언, 미니언 등) 검색
	for (AActor* Actor : FoundActors)
	{
		if (Actor == this || !Actor) continue;
        
		if (Actor->ActorHasTag(TEXT("Character")) && !Actor->ActorHasTag(MyTeamTag)) 
		{
			float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				BestTarget = Actor;
			}
		}
	}
	// 2순위: 주변에 적 유닛이 없다면 적 넥서스 검색
	if (BestTarget == nullptr)
	{
		TArray<AActor*> NexusActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANexus::StaticClass(), NexusActors);

		for (AActor* Nexus : NexusActors)
		{
			// 넥서스에도 팀 태그가 있어야 작동
			if (Nexus && !Nexus->ActorHasTag(MyTeamTag))
			{
				BestTarget = Nexus;
				break; // 넥서스는 보통 하나이므로 찾으면 바로 중단
			}
		}
	}
	
	// 이전 타겟과 새로 찾은 타겟이 다르면 경로 초기화 하고 새로 길을 찾아야 함
	if (TargetPlayer != BestTarget)
	{
		CurrentPath.Empty();
		TargetPlayer = BestTarget;
	}
}