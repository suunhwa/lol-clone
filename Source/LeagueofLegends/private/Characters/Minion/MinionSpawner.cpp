#include "Characters/Minion/MinionSpawner.h"
#include "Characters/LoLMinion.h"
#include "Manager/MinionDataSubsystem.h" // 블루프린트 클래스 매핑용
#include "GameFramework/GameStateBase.h"
#include "TimerManager.h"
#include "Components/TagComponent.h"

AMinionSpawner::AMinionSpawner()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AMinionSpawner::BeginPlay()
{
    Super::BeginPlay();

    if (!HasAuthority()) return;

    // 롤 기준 첫 웨이브 1분 5초(65초) 시작, 이후 30초 간격
    // 지금은 테스트용으로 5초에 시작, 10초 간격
    GetWorldTimerManager().SetTimer(WaveTimerHandle, this, &AMinionSpawner::CheckAndSpawnWave, 
        10.f, true, 5.f);
}

void AMinionSpawner::CheckAndSpawnWave()
{
    if (!WaveDataTable) 
    {
        UE_LOG(LogTemp, Error, TEXT("MinionSpawner: 데이터 테이블이 할당되지 않았습니다!"));
        return;
    }

    CurrentWaveCount++;

    // 현재 서버 시간 기준 데이터 로우 선택
    float CurrentTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
    FMinionWaveRow* TargetRow = nullptr;

    TArray<FMinionWaveRow*> AllRows;
    WaveDataTable->GetAllRows<FMinionWaveRow>(TEXT("WaveSearch"), AllRows);

    for (auto* Row : AllRows)
    {
        if (CurrentTime >= Row->Start_Time && CurrentTime < Row->End_Time)
        {
            TargetRow = Row;
            break;
        }
    }

    if (!TargetRow) return;

    // CSV 조건에 따른 스폰 리스트 결정
    FString SelectedIDList;

    if (bIsInhibitorDestroyed)
    {
        SelectedIDList = TargetRow->Alt_Spawn_List; // 슈퍼 미니언 포함
    }
    else if (CurrentWaveCount % TargetRow->Siege_Cycle == 0)
    {
        SelectedIDList = TargetRow->Special_Spawn_List; // 대포 미니언 포함
    }
    else
    {
        SelectedIDList = TargetRow->Normal_Spawn_List; // 일반 미니언 구성
    }

    // 스폰 실행
    ExecuteSpawnSequence(SelectedIDList);
}

void AMinionSpawner::ExecuteSpawnSequence(const FString& IDListString)
{
    TArray<int32> MinionIDs = ParseIDList(IDListString);

    for (int32 i = 0; i < MinionIDs.Num(); ++i)
    {
        int32 TargetID = MinionIDs[i];
        FTimerHandle DelayHandle;

        GetWorldTimerManager().SetTimer(DelayHandle, [this, TargetID]()
        {
            if (MinionClassMap.Contains(TargetID))
            {
                TSubclassOf<ALoLMinion> MinionClass = MinionClassMap[TargetID];
                FActorSpawnParameters Params;
                Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

                ALoLMinion* SpawnedMinion = GetWorld()->SpawnActor<ALoLMinion>(MinionClass, GetActorLocation(), GetActorRotation(), Params);
                
                if (SpawnedMinion)
                {
                    // 팀 설정 주입 (TagComp뿐만 아니라 미니언 멤버 변수에도 직접)
                    // 만약 InitialTeam 변수가 있다면 그것도 채워줍니다.
                    // SpawnedMinion->InitialTeam = SpawnerTeam; 

                    if (UTagComponent* TagComp = SpawnedMinion->FindComponentByClass<UTagComponent>())
                    {
                        TagComp->SetTeam(SpawnerTeam);
                    }

                    // 스폰 직후 즉시 타겟을 찾도록 강제 호출
                    // 미니언이 BeginPlay에서 적이 없어 포기했을 수 있으므로 다시 깨웁니다.
                    SpawnedMinion->SetActorTickEnabled(true);
                    
                    UE_LOG(LogTemp, Warning, TEXT("[%s-%s] ID %d 스폰 및 팀 주입 완료!"), 
                        *UEnum::GetValueAsString(SpawnerTeam), *UEnum::GetValueAsString(SpawnerLane), TargetID);
                }
            }
        }, i * 0.5f, false);
    }
}

TArray<int32> AMinionSpawner::ParseIDList(const FString& IDListString)
{
    TArray<int32> Result;
    FString CleanString = IDListString.Replace(TEXT("("), TEXT("")).Replace(TEXT(")"), TEXT(""));
    
    TArray<FString> IDStrings;
    CleanString.ParseIntoArray(IDStrings, TEXT(","), true);

    for (const FString& S : IDStrings)
    {
        Result.Add(FCString::Atoi(*S.TrimStartAndEnd()));
    }
    return Result;
}