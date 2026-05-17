#include "Characters/Minion/MinionSpawner.h"

#include "LeagueofLegends.h"
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
	
	// 시작하자마자 스폰하고 싶으면 추후 StartWaveTimer) 호출

	if (HasAuthority())
	{
		StartWaveTimer();
	}
	/*// 키 바인딩 --------------------------------------- 테스트 후 추후 삭제 ------------------------
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		this->EnableInput(PC);

		if (InputComponent)
		{
			// N 키 바인딩
			FInputKeyBinding& Binding9 = InputComponent->BindKey(EKeys::Nine,
			                                                     IE_Pressed,
			                                                     this,
			                                                     &AMinionSpawner::OnPress9Key);
			Binding9.bConsumeInput = false; // 중요: 입력을 소비하지 않음 (다른 스포너도 받을 수 있게)

			// M 키 바인딩
			FInputKeyBinding& Binding0 = InputComponent->BindKey(EKeys::Zero,
			                                                     IE_Pressed,
			                                                     this,
			                                                     &AMinionSpawner::OnPress0Key);
			Binding0.bConsumeInput = false; // 중요: 입력을 소비하지 않음
		}
	}
	// ------------------------------------ 요기까지 삭제 -----------------------------*/

	
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

	// 로그로 개수 재확인
	PRINTLOG_HJ(TEXT("파싱된 총 미니언 수: %d, 원본 문자열: %s"), MinionIDs.Num(), *IDListString);

	for (int32 i = 0; i < MinionIDs.Num(); ++i)
	{
		int32 TargetID = MinionIDs[i];

		// [핵심 수정] 루프마다 완전히 독립적인 핸들을 사용하도록 보장
		FTimerHandle* NewHandle = new FTimerHandle();

		// 0.5초 간격으로 스폰하되, 블루/레드 팀 간의 타이머 충돌을 피하기 위해 
		// 미세한 오프셋(i * 0.001f)을 추가하는 것이 안전해.
		float StartDelay = (i * 0.5f) + 0.01f;

		GetWorldTimerManager().SetTimer(*NewHandle,
		                                [this, TargetID, i, NewHandle]()
		                                {
			                                if (MinionClassMap.Contains(TargetID))
			                                {
				                                // 1. 포메이션 좌표 계산
				                                FVector SpawnLocation = GetActorLocation();
				                                float ForwardOffset = (i / 3) * -150.f;
				                                float SideOffset = ((i % 3) - 1) * 100.f;

				                                FVector Offset = (GetActorForwardVector() * ForwardOffset) + (
					                                GetActorRightVector() * SideOffset);
				                                FVector FinalLocation = SpawnLocation + Offset;

				                                // 2. 소환 옵션 (AlwaysSpawn 적용됨)
				                                FActorSpawnParameters Params;
				                                Params.SpawnCollisionHandlingOverride =
					                                ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				                                TSubclassOf<ALoLMinion> MinionClass = MinionClassMap[TargetID];

				                                // ALoLMinion* SpawnedMinion = GetWorld()->SpawnActor<ALoLMinion>(MinionClass, FinalLocation, GetActorRotation(), Params);

				                                // SpawnActorDeferred: BeginPlay 전에 팀을 먼저 세팅
				                                FTransform SpawnTransform(GetActorRotation(), FinalLocation);
				                                ALoLMinion* SpawnedMinion = GetWorld()->SpawnActorDeferred<ALoLMinion>(
					                                MinionClass,
					                                SpawnTransform,
					                                nullptr,
					                                nullptr,
					                                ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

				                                if (SpawnedMinion)
				                                {
					                                // 팀 설정 주입
				                                	// BeginPlay 실행 전에 팀 세팅 → FOW 초기화 시 팀이 이미 확정
					                                if (UTagComponent* TagComp = SpawnedMinion->FindComponentByClass<
						                                UTagComponent>())
					                                {
						                                TagComp->SetTeam(SpawnerTeam);
					                                }
				                                	
				                                	SpawnedMinion->FinishSpawning(SpawnTransform);
				                                	
					                                // 경로 주입
					                                if (Waypoints.Num() > 0)
					                                {
						                                TArray<FVector> WorldWaypoints;
						                                for (const FVector& LocalPoint : Waypoints)
						                                {
							                                WorldWaypoints.Add(
								                                GetActorTransform().TransformPosition(LocalPoint));
						                                }
						                                SpawnedMinion->SetLanePath(WorldWaypoints);
					                                }

					                                SpawnedMinion->SetActorTickEnabled(true);

					                                PRINTLOG_HJ(TEXT("[%s] ID %d 스폰 완료 (인덱스: %d)"),
					                                            *UEnum::GetValueAsString(SpawnerTeam),
					                                            TargetID,
					                                            i);
				                                }
			                                }

			                                // 사용이 끝난 동적 할당 핸들 메모리 해제
			                                delete NewHandle;
		                                },
		                                StartDelay,
		                                false);
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

void AMinionSpawner::StartWaveTimer()
{
	if (!HasAuthority()) return;

	// 롤 기준 첫 웨이브 1분 5초(65초) 시작, 이후 30초 간격
	GetWorldTimerManager().SetTimer(WaveTimerHandle,
	                                this,
	                                &AMinionSpawner::CheckAndSpawnWave,
	                                30.f,
	                                true,
	                                65.f);
	PRINTLOG_HJ(TEXT("미니언 웨이브 생성이 시작되었습니다."));
}

void AMinionSpawner::StopWaveTimer()
{
	GetWorldTimerManager().ClearTimer(WaveTimerHandle);
	PRINTLOG_HJ(TEXT("미니언 웨이브 생성이 완전히 중지되었습니다."));
}

void AMinionSpawner::SetWavePaused(bool bPaused)
{
	if (bPaused)
		GetWorldTimerManager().PauseTimer(WaveTimerHandle);
	else
		GetWorldTimerManager().UnPauseTimer(WaveTimerHandle);
}

bool AMinionSpawner::IsWaveTimerActive() const
{
	return GetWorldTimerManager().IsTimerActive(WaveTimerHandle);
}

/*// 테스트 종료후 여기도 삭제 ------------------------------
void AMinionSpawner::OnPress9Key()
{
	StartWaveTimer();
	// 화면에 즉각적으로 피드백 표시 (선택 사항)
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("9 Key Pressed: Wave Started"));
	}
}

// M키 눌렀을 때 실행될 래퍼 함수
void AMinionSpawner::OnPress0Key()
{
	StopWaveTimer();
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("0 Key Pressed: Wave Stopped"));
	}
}

// ------------------- 여기까지 두 함수 삭제하면됨 헤더도 잊지말고 삭제*/
