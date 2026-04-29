#include "Characters/LoLMinion.h"
#include "Kismet/GameplayStatics.h"
#include "AStar/AStarGridManager.h"
#include "Characters/Nexus/Nexus.h"
#include "Components/CapsuleComponent.h"

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
    GridManager = Cast<AAStarGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AAStarGridManager::StaticClass()));
}

void ALoLMinion::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // 죽었거나 때릴 대상이 아예 없으면 아무것도 하지 않는다
    if (CurrentState == EMinionState::Dead || !TargetPlayer) return;
    if (!TargetPlayer) return;
    // 나와 타겟 사이의 거리를 계산한다
    float DistanceToTarget = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());

    // 상태 결정 로직
    // 거리가 사거리 보다 가까우면
    if (DistanceToTarget <= AttackRange)  
    {
       CurrentState = EMinionState::Attacking; // 공격
       CurrentPath.Empty(); 
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
       SetActorRotation(Direction.Rotation());
       PerformAttack();
    }
    else if (CurrentState == EMinionState::MoveToTarget)
    {
       MoveAlongPath(DeltaTime); // 길 따라서 걷기 함수 실행
    }

    // [디버그 로그] 미니언이 노리는 상대방을 노란 선으로 보여줌
    if (TargetPlayer) {
        DrawDebugLine(GetWorld(), GetActorLocation(), TargetPlayer->GetActorLocation(), FColor::Yellow, false, 0.1f, 0, 1.0f);
    }
}

void ALoLMinion::TakeDamageSimple(float Damage)
{
   if (CurrentState == EMinionState::Dead) return;

   HP -= Damage;
   UE_LOG(LogTemp, Log, TEXT("[%s] 피격! 남은 체력: %.1f"), *GetName(), HP);

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
      MinionID        = BaseData->MinionID;
      MoveSpeed       = BaseData->MoveSpeed;
      AttackRange     = BaseData->AtkRange;
      AttackSpeed     = BaseData->AtkSpeed;
      ProjSpeed       = BaseData->ProjSpeed; // 원거리에서 쓸 변수
      Armor           = BaseData->Armor;
      MagicResistance = BaseData->MR;
      CollisionRadius = BaseData->Collision;
      bIsSiege        = BaseData->Is_Siege;
      bIsSuper        = BaseData->Is_Super;
      TowerDamageReduction = BaseData->Tower_DR;
      Name_KR         = BaseData->Name_KR;

      // [캡슐 컴포넌트 크기 동적 설정]
      // 테이블의 Collision 값에 따라 미니언의 물리적 크기가 결정됩니다.
      if (UCapsuleComponent* Capsule = GetCapsuleComponent())
      {
         Capsule->SetCapsuleRadius(CollisionRadius);
      }
        
      // [GrowthTable 성장 스탯 계산]
      float CurrentGameTime = GetWorld()->GetTimeSeconds();
      int32 GrowthCycle = FMath::Min(FMath::FloorToInt(CurrentGameTime / GrowthData->Interval), GrowthData->Max_Cycle);

      HP = GrowthData->Base_HP + (GrowthData->HP_Up * GrowthCycle);
      AttackDamage = GrowthData->Base_AD + (GrowthData->AD_Up * GrowthCycle);

      UE_LOG(LogTemp, Log, TEXT("[%s] 데이터 로드 완료: HP %.1f, AD %.1f"), *Name_KR, HP, AttackDamage);
   }
}

void ALoLMinion::UpdateTarget()
{
    // 월드에서 'Character' 태그를 가진 모든 액터를 찾는다
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("Character"), FoundActors);

    AActor* BestTarget = nullptr;
    float ClosestDistance = MAX_FLT;
    // 내 팀 태그(팀 태그는 Tag[0] 인덱스에 쓰도록 한다. RedTeam/BlueTeam/GreenTeam ....
    FName MyTeamTag = (Tags.Num() > 0) ? Tags[0] : NAME_None;
    // 상대 팀 태그 결정
    FName EnemyTeamTag = (MyTeamTag == TEXT("RedTeam")) ? TEXT("BlueTeam") : TEXT("RedTeam");
   
    // 1순위 : 주변 적 유닛 중 가장 가까운 대상을 찾는다
    for (AActor* Actor : FoundActors)
    {
       if (!Actor || Actor == this) continue; // 나 자신은 제외
       // 'Character' 태그가 잇어도 내 팀 태그와 다르면 적으로 간주
       if (Actor->ActorHasTag(TEXT("Character")) && Actor->ActorHasTag(EnemyTeamTag))
       {
          float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
          if (Distance < ClosestDistance)
          {
             ClosestDistance = Distance;
             BestTarget = Actor;
          }
       }
    }

    // 2순위: 주변에 적 유닛이 하나도 없다면, 적팀의 넥서스를 찾는다.
    if (!BestTarget)
    {
       TArray<AActor*> NexusActors;
       UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANexus::StaticClass(), NexusActors);
       for (AActor* Nexus : NexusActors)
       {
          if (Nexus && !Nexus->ActorHasTag(MyTeamTag)) // 적팀 넥서스 발견
          {
             BestTarget = Nexus;
             break;
          }
       }
    }
    
    // 타겟이 바뀌었을 때만 기존 경로를 지운다
    if (TargetPlayer != BestTarget)
    {
       UE_LOG(LogTemp, Warning, TEXT("[%s] 타겟 변경됨 -> %s"), *GetName(), BestTarget ? *BestTarget->GetName() : TEXT("None"));
       TargetPlayer = BestTarget;
       CurrentPath.Empty(); // 새 타겟을 위한 새 길을 찾기 위해 비워둔다
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
       CurrentPath = GridManager->FindPath(GetActorLocation(), TargetPlayer->GetActorLocation());
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
          FVector Direction = (NextPoint - MyLoc).GetSafeNormal2D();
          SetActorLocation(MyLoc + (Direction * MoveSpeed * DeltaTime));
          
          FRotator TargetRot = Direction.Rotation();
          SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, 10.0f));
       }
    }
}