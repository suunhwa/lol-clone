#include "Characters/LoLMinion.h"

#include "LeagueofLegends.h"
#include "Kismet/GameplayStatics.h"
#include "AStar/AStarGridManager.h"
#include "Characters/LoLStructure.h"
#include "Characters/Nexus/Nexus.h"
#include "Components/CapsuleComponent.h"
#include "Engine/OverlapResult.h"

ALoLMinion::ALoLMinion()
{
    PrimaryActorTick.bCanEverTick = true;

    // [스탯 설정] 임의로 테스트용
    HP = 0.0f;
    AttackDamage = 0.0f;
    AttackSpeed = 0.0f; // 초당 1회 공격
    AttackRange = 0.0f;
    MoveSpeed = 0.0f;
}

void ALoLMinion::BeginPlay()
{
    Super::BeginPlay();
    InitializeStatsFromTable();

    GetWorldTimerManager().SetTimer(TargetUpdateTimerHandle, this, &ALoLMinion::UpdateTarget, 0.5f, true);
    GridManager = Cast<AAStarGridManager>(
       UGameplayStatics::GetActorOfClass(GetWorld(), AAStarGridManager::StaticClass()));
}

void ALoLMinion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// 죽었거나 때릴 대상이 아예 없으면 아무것도 하지 않는다
	if (CurrentState == EMinionState::Dead) return;
	if (!TargetPlayer) 
	{
		// 1초에 한 번만 출력되도록 제한
		static float LastLogTime = 0;
		if (GetWorld()->GetTimeSeconds() - LastLogTime > 1.0f)
		{
			PRINTLOG_HJ(LogTemp, Warning, TEXT("[%s] 현재 타겟 없음! 대기 중..."), *GetName());
			LastLogTime = GetWorld()->GetTimeSeconds();
		}
		
		return;
	}
	
    // 나와 타겟 사이의 거리를 계산한다
    float DistanceToTarget = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());

    // 상태 결정 로직
    // 거리가 사거리 보다 가까우면
    if (DistanceToTarget <= AttackRange)
    {
       CurrentState = EMinionState::Attacking; // 공격
       CurrentPath.Empty(); // 공격 시작시 경로 제거
    }
    // 거리가 사거리 보다 멀면
    else
    {
       CurrentState = EMinionState::MoveToTarget; // 이동
    }

    // 현재 상태가 공격이면
    if (CurrentState == EMinionState::Attacking)
    {
       // 타겟쪽으로 몸을 돌리고 공격 함수 실행
       FVector Direction = TargetPlayer->GetActorLocation() - GetActorLocation();
       Direction.Z = 0.0f;
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), Direction.Rotation(), DeltaTime, 10.0f));
       PerformAttack();
    }
    else if (CurrentState == EMinionState::MoveToTarget)
    {
       MoveAlongPath(DeltaTime); // 길 따라서 걷기 함수 실행
    }

    // [디버그 로그] 미니언이 노리는 상대방을 노란 선으로 보여줌
    if (TargetPlayer)
    {
       DrawDebugLine(GetWorld(), GetActorLocation(), TargetPlayer->GetActorLocation(), FColor::Yellow, false, 0.1f, 0,
                     1.0f);
    }
}

void ALoLMinion::TakeDamageSimple(float Damage)
{
    if (CurrentState == EMinionState::Dead) return;

    HP -= Damage;
    PRINTLOG_HJ(LogTemp, Log, TEXT("[%s] 피격! 남은 체력: %.1f"), *GetName(), HP);

    if (HP <= 0.0f)
    {
       HP = 0.0f;
       CurrentState = EMinionState::Dead;

       UE_LOG(LogTemp, Error, TEXT("[%s] 처치됨!"), *GetName());

       // 사망 시 바로 파괴하거나, 시체가 남길 원하면 시간을 둡니다.
       // 여기서는 사태 진압을 위해 0.1초 뒤 즉시 제거합니다.
       SetLifeSpan(0.1f);
    }
}

void ALoLMinion::InitializeStatsFromTable()
{
    UMinionDataSubsystem* DataSubsystem = GetGameInstance()->GetSubsystem<UMinionDataSubsystem>();
    if (!DataSubsystem) return;

    int32 TargetID = FCString::Atoi(*MinionDataID.ToString());

    FMinionBaseRow* BaseData = DataSubsystem->GetBaseRowByID(TargetID);
    FMinionGrowthRow* GrowthData = DataSubsystem->GetGrowthRowByID(TargetID);

    if (BaseData && GrowthData)
    {
       // [BaseTable 정보 로드]
       MinionID = BaseData->MinionID;
       MoveSpeed = BaseData->MoveSpeed;
       AttackRange = BaseData->AtkRange;
       AttackSpeed = BaseData->AtkSpeed;
       ProjSpeed = BaseData->ProjSpeed; // 원거리에서 쓸 변수
       Armor = BaseData->Armor;
       MagicResistance = BaseData->MR;
       CollisionRadius = BaseData->Collision;
       bIsSiege = BaseData->Is_Siege;
       bIsSuper = BaseData->Is_Super;
       TowerDamageReduction = BaseData->Tower_DR;
       Name_KR = BaseData->Name_KR;

       // [캡슐 컴포넌트 크기 동적 설정]
       // 테이블의 Collision 값에 따라 미니언의 물리적 크기가 결정됩니다.
       if (UCapsuleComponent* Capsule = GetCapsuleComponent())
       {
          Capsule->SetCapsuleRadius(CollisionRadius);
       }

       // [GrowthTable 성장 스탯 계산]
       float CurrentGameTime = GetWorld()->GetTimeSeconds();
       int32 GrowthCycle = FMath::Min(FMath::FloorToInt(CurrentGameTime / GrowthData->Interval),
                                      GrowthData->Max_Cycle);

       HP = GrowthData->Base_HP + (GrowthData->HP_Up * GrowthCycle);
       AttackDamage = GrowthData->Base_AD + (GrowthData->AD_Up * GrowthCycle);

       PRINTLOG_HJ(LogTemp, Log, TEXT("[%s] 데이터 로드 완료: HP %.1f, AD %.1f"), *Name_KR, HP, AttackDamage);
    }
}

void ALoLMinion::UpdateTarget()
{
    // 1. 내 팀 및 적 팀 태그 판별 (인덱스 의존 탈피)
    FName MyTeamTag = Tags.Contains(TEXT("RedTeam")) ? TEXT("RedTeam") : TEXT("BlueTeam");
    FName EnemyTeamTag = (MyTeamTag == TEXT("RedTeam")) ? TEXT("BlueTeam") : TEXT("RedTeam");

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("Character"), FoundActors);

    AActor* BestUnitTarget = nullptr;   // 1순위: 적 미니언
    AActor* BestBuildingTarget = nullptr; // 2순위: 적 구조물
    
    float ClosestUnitDist = MAX_FLT;
    float ClosestBuildingDist = MAX_FLT;

    for (AActor* Actor : FoundActors)
    {
        if (!Actor || Actor == this) continue;
    	
    	if (!UKismetSystemLibrary::IsValid(Actor)) continue;
        // 적 팀 태그를 가졌는지 확인
        if (Actor->ActorHasTag(EnemyTeamTag))
        {
            float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());

            // [우선순위 판별] 미니언인지 구조물인지 클래스로 구분
            if (Actor->IsA(ALoLMinion::StaticClass())) 
            {
                if (Distance < ClosestUnitDist)
                {
                    ClosestUnitDist = Distance;
                    BestUnitTarget = Actor;
                }
            }
            else if (Actor->IsA(ALoLStructure::StaticClass()))
            {
                if (Distance < ClosestBuildingDist)
                {
                    ClosestBuildingDist = Distance;
                    BestBuildingTarget = Actor;
                }
            }
        }
    }

    // 미니언이 하나라도 있으면 미니언을 타겟으로, 없으면 구조물을 타겟으로 설정
    AActor* NewTarget = BestUnitTarget ? BestUnitTarget : BestBuildingTarget;

    // 타겟이 실제로 바뀌었을 때만 경로 초기화
    if (TargetPlayer != NewTarget)
    {
    	FString OldTargetName = TargetPlayer ? TargetPlayer->GetName() : TEXT("None");
    	FString NewTargetName = NewTarget ? NewTarget->GetName() : TEXT("None");
    	PRINTLOG_HJ(LogTemp, Log, TEXT("[%s] 타겟 변경: %s -> %s"), *GetName(), *OldTargetName, *NewTargetName);
    	
        TargetPlayer = NewTarget;
        CurrentPath.Empty(); 
    }
}

// 암튼 이거 공격 로직인데 데이터 불러온거도 없어서 야매로 해서 땜빵침 나중에 대대적인 수정
void ALoLMinion::PerformAttack()
{
    if (!TargetPlayer) return;
    float CurrentTime = GetWorld()->GetTimeSeconds();

    if (CurrentTime - LastAttackTime >= (1.0f / AttackSpeed))
    {
       // 1. 인터페이스 방식으로 데미지 전달 (가장 권장되는 방식)
       IDamageable* DamageTarget = Cast<IDamageable>(TargetPlayer);
       if (DamageTarget)
       {
	       // [로그 추가] 미니언이 실제로 대미지를 주려고 시도하는지 확인
	       UE_LOG(LogTemp, Log, TEXT("[%s] %s에게 공격 시도 (AD: %.1f)"),
	              *GetName(), *TargetPlayer->GetName(), AttackDamage);
       	
          // 미니언/구조물 구분 없이 데미지 입힘
          DamageTarget->ReceiveDamage(AttackDamage, EDamageType::Physical, this);
       }

       LastAttackTime = CurrentTime;
    }
}

void ALoLMinion::MoveAlongPath(float DeltaTime)
{
    if (!GridManager || !TargetPlayer) return;
    // 가야 할 경로가 비어있으면 매니저한테 길을 물어봄
    if (CurrentPath.Num() == 0)
    {
    	// 로그 3: 경로 탐색 시도
    	PRINTLOG_HJ(LogTemp, Log, TEXT("[%s] 경로가 비어있음. %s 방향으로 경로 탐색 시도"), *GetName(), *TargetPlayer->GetName());
    	CurrentPath = GridManager->FindPath(GetActorLocation(), TargetPlayer->GetActorLocation());
       
    	if (CurrentPath.Num() == 0)
    	{
    		PRINTLOG_HJ(LogTemp, Error, TEXT("[%s] 경로 탐색 실패! (갈 수 없는 지역이거나 타겟이 격자 밖임)"), *GetName());
    	}
    	else 
    	{
    		PRINTLOG_HJ(LogTemp, Log, TEXT("[%s] 경로 탐색 성공: %d개 지점 발견"), *GetName(), CurrentPath.Num());
    	}
    	
    }
    // 경로가 있으면
    if (CurrentPath.Num() > 0)
    {
       FVector NextPoint = CurrentPath[0]; // 가야 할 첫번째 지점
       FVector MyLoc = GetActorLocation();

       // 경로 시각화, 노란선
       DrawDebugLine(GetWorld(), MyLoc, NextPoint, FColor::Yellow, false, 0.1f, 0, 5.0f);
       // 바닥 기준으로 다음 지점까지 거리 계산
       float DistToPoint = FVector2D::Distance(FVector2D(MyLoc.X, MyLoc.Y), FVector2D(NextPoint.X, NextPoint.Y));

       if (DistToPoint < 45.0f) // 지점에 충분히 가까워졌으면
       {
          CurrentPath.RemoveAt(0); // 목록에서 지우고 다음 지점으로 이동
       }
       else // 가깝지 않으면 계속이동
       {
          MyLoc = GetActorLocation();
          FVector PathDirection = (NextPoint - MyLoc).GetSafeNormal2D();
    
          // --- [떨림 방지: 회피 벡터 계산] ---
          FVector AvoidanceVector = FVector::ZeroVector;
          TArray<FOverlapResult> Overlaps;
          // 내 충돌 범위보다 살짝 큰 영역을 검사 (CollisionRadius가 50이면 60~70 정도)
          FCollisionShape Scope = FCollisionShape::MakeSphere(CollisionRadius * 1.2f); 
          FCollisionQueryParams Params;
          Params.AddIgnoredActor(this);

          // 주변에 다른 미니언이 있는지 체크
          if (GetWorld()->OverlapMultiByChannel(Overlaps, MyLoc, FQuat::Identity, ECC_Pawn, Scope, Params))
          {
             for (const FOverlapResult& Overlap : Overlaps)
             {
                if (Overlap.GetActor() && Overlap.GetActor()->ActorHasTag(TEXT("Character")))
                {
                   // 상대방에게서 멀어지는 방향 벡터
                   FVector PushDir = MyLoc - Overlap.GetActor()->GetActorLocation();
                   float Dist = PushDir.Size();
                   if (Dist > 0.0f)
                   {
                      // 가까울수록 더 강하게 밀어냄 (가중치 0.5~1.0 사이에서 조절)
                      AvoidanceVector += (PushDir.GetSafeNormal() / Dist) * 5.0f;
                   }
                }
             }
          }

          // 최종 방향 = (경로 방향 + 회피 방향)을 합쳐서 계산
          FVector FinalDirection = (PathDirection + AvoidanceVector).GetSafeNormal2D();

          // 위치 업데이트 (FinalDirection 사용)
          SetActorLocation(MyLoc + (FinalDirection * MoveSpeed * DeltaTime));
    
          // 회전은 떨림 방지를 위해 원래 가야 할 '경로 방향'을 유지
          FRotator TargetRot = PathDirection.Rotation();
          SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, 10.0f));
       }
    }
}

// 포탑이 호출하는 ReceiveDamage를 미니언의 HP 로직으로 연결
void ALoLMinion::ReceiveDamage(float Amount, EDamageType DamageType, AActor* DamageInstigator)
{
	// 부모 클래스의 HasAuthority() 체크를 우회하여 즉시 대미지 적용
	// 미니언 클래스에 이미 만들어둔 TakeDamageSimple 함수를 호출합니다.
	TakeDamageSimple(Amount);
}

// 태그를 기반으로 정확한 팀 정보를 반환
ETeam ALoLMinion::GetTeam() const
{
	if (Tags.Contains(TEXT("RedTeam"))) return ETeam::Red;
	if (Tags.Contains(TEXT("BlueTeam"))) return ETeam::Blue;
	return ETeam::None;
}