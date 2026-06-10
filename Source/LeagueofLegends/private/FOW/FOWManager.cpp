// Fill out your copyright notice in the Description page of Project Settings.


#include "FOW/FOWManager.h"

#include "LeagueofLegends.h"
#include "Characters/LoLCharacterBase.h"
#include "Interfaces/SightProviderHelper.h"
#include "FOW/FOWTileMap.h"
#include "GameFramework/RiftGameState.h"
#include "Kismet/GameplayStatics.h"

#define TEST 0

// Sets default values
AFOWManager::AFOWManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AFOWManager::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (auto* GS = GetWorld()->GetGameState<ARiftGameState>())
	{
		GS->SetFOWManager(this);
	}
	else
	{
		PRINTLOG_TK(TEXT("FOWManager: GameState is not ARiftGameState!"));
	}
}

void AFOWManager::BroadcastFOWReadyIfValid()
{
	if (AFOWTileMap* LocalTileMap = GetLocalTileMap())
	{
		if (UTexture2D* Tex = LocalTileMap->GetFogTexture())
		{
			OnFOWReady.Broadcast(Tex);
		}
	}
}

bool AFOWManager::IsLocalTileMapReady() const
{
	AFOWTileMap* TM = GetLocalTileMap();
	return TM && TM->GetFogTexture() != nullptr;
}

// Called when the game starts or when spawned
void AFOWManager::BeginPlay()
{
	Super::BeginPlay();

	if (!FOWVolume)
	{
		return;
	}

	if (HasAuthority())
	{
		// 양 팀 시야 판정용 타일 데이터 미리 생성
		RedTileMap->GenerateTileData(FOWVolume);
		BlueTileMap->GenerateTileData(FOWVolume);
	}
	// 클라이언트 경로는 BeginPlay에서 아무것도 하지 않음
	// (SetLocalClientTeam에서 처리됨)

#if TEST
	// 현재 playerPawn을 TestActor로 할당
	TestActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
#endif
}

void AFOWManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
#if TEST
	UpdateFOV(TestTileMap, RedSightProviders); // 테스트용
#else
	if (HasAuthority())
	{
		UpdateFOV(RedTileMap, RedSightProviders, false);
		UpdateFOV(BlueTileMap, BlueSightProviders, false);

		UpdateEnemyVisibility_Server(RedTileMap, BlueSightProviders, ERiftSightTag::Red);
		UpdateEnemyVisibility_Server(BlueTileMap, RedSightProviders, ERiftSightTag::Blue);

		// 호스트의 로컬 팀 비주얼 갱신 (팀 미정 또는 비주얼 미준비면 스킵)
		if (LocalClientTeam != ERiftSightTag::None && IsLocalTileMapReady())
		{
			GetLocalTileMap()->UpdateFogTexture();
		}
	}
	else
	{
		// 클라이언트: 팀 미정 또는 비주얼 미준비면 스킵
		if (LocalClientTeam == ERiftSightTag::None || !IsLocalTileMapReady())
		{
			return;
		}

		AFOWTileMap* LocalTM = GetLocalTileMap();
		TArray<TScriptInterface<ISightProvider>>& Providers =
			(LocalClientTeam == ERiftSightTag::Red) ? RedSightProviders : BlueSightProviders;
		UpdateFOV(LocalTM, Providers, true);
	}
#endif
}

void AFOWManager::UpdateFOV(AFOWTileMap* TileMap, TArray<TScriptInterface<ISightProvider>>& SightProviders, 
							bool bUpdateTexture)
{
	if (!TileMap) return;

#if TEST
	if (!TestActor) return;

	TileMap->ResetTileVisibility();

	FIntPoint Origin = TileMap->WorldToTile(TestActor->GetActorLocation());
	int32 MaxDepth = FMath::FloorToInt(1000.f / TileMap->GetTileSize());
	ComputeFOV(Origin, TileMap, MaxDepth);

	TileMap->UpdateFogTexture();
#else
	if (SightProviders.Num() == 0) return;

	TileMap->ResetTileVisibility();

	for (const TScriptInterface<ISightProvider>& Provider : SightProviders)
	{
		FVector Origin3D = SightProviderHelper::GetSightOrigin(Provider.GetObject());
		FIntPoint Origin = TileMap->WorldToTile(Origin3D);

		float SightRange = SightProviderHelper::GetSightRange(Provider.GetObject());
		int32 MaxDepth = FMath::FloorToInt(SightRange / TileMap->GetTileSize());
		ComputeFOV(Origin, TileMap, MaxDepth);
	}

	if (bUpdateTexture)
	{
		TileMap->UpdateFogTexture();
		// TileMap->UpdateSightDataTexture(SightProviders);
	}
#endif
}

void AFOWManager::RegisterSightProvider(UObject* SightObject)
{
	if (!SightObject || !SightProviderHelper::IsSightProvider(SightObject))
	{
		PRINTLOG_TK(TEXT("RegisterSightProvider: Invalid SightObject"));
		return;
	}

	TScriptInterface<ISightProvider> SightProvider;
	SightProvider.SetObject(SightObject);
	SightProvider.SetInterface(SightProviderHelper::TryGetProvider(SightObject));
	
	RedSightProviders.Remove(SightProvider);
	BlueSightProviders.Remove(SightProvider);

	const ERiftSightTag Team = SightProviderHelper::GetTeam(SightObject);
	if (Team == ERiftSightTag::Red)
	{
		RedSightProviders.Add(SightProvider);
	}
	else if (Team == ERiftSightTag::Blue)
	{
		BlueSightProviders.Add(SightProvider);
	}
	else
	{
		PRINTLOG_TK(TEXT("RegisterSightProvider: SightObject %s has invalid team tag"), *SightObject->GetName());
	}
}

void AFOWManager::UnregisterSightProvider(UObject* SightObject)
{
	if (!SightObject || !SightProviderHelper::IsSightProvider(SightObject))
	{
		UE_LOG(LogTemp, Warning, TEXT("UnregisterSightProvider: Invalid object"));
		return;
	}

	TScriptInterface<ISightProvider> SightProvider;
	SightProvider.SetObject(SightObject);
	SightProvider.SetInterface(SightProviderHelper::TryGetProvider(SightObject));

	RedSightProviders.Remove(SightProvider);
	BlueSightProviders.Remove(SightProvider);
}

void AFOWManager::ComputeFOV(const FIntPoint& Origin, AFOWTileMap* TileMap, int32 MaxDepth)
{
	TileMap->SetTileVisibility(Origin.X, Origin.Y, true); // 플레이어 위치는 항상 보이도록 설정
	
	// 4분면 각각에 대해 Scan 호출
	for (int32 i = 0; i < 4; i++)
	{
		FQuadrant Quadrant{ static_cast<EQuadrantDirection>(i), Origin };
		Scan(FRow(), Quadrant, TileMap, Origin, MaxDepth);
	}
}

void AFOWManager::Scan(FRow Row, const FQuadrant& Quadrant, AFOWTileMap* TileMap, const FIntPoint& Origin, int32 MaxDepth)
{
	if (Row.Depth > MaxDepth)
	{
		return;
	}
	
	bool bHasPrev = false;
	FIntPoint PrevTilePoint;
	for (int32 Col = Row.GetMinCol(); Col <= Row.GetMaxCol(); Col++)
	{
		FIntPoint CurTilePoint{ Row.Depth, Col };
		if (IsWall(CurTilePoint, Quadrant, TileMap) ||
			IsSymmetric(Row, Row.Depth, Col))
		{
			Reveal(CurTilePoint, Quadrant, TileMap, Origin, MaxDepth);
		}
		
		if (bHasPrev)
		{
			if (IsWall(PrevTilePoint, Quadrant, TileMap) && IsFloor(CurTilePoint, Quadrant, TileMap))
			{
				Row.StartSlop = FFraction{ Col * 2 - 1, Row.Depth * 2 };
			}
			else if (IsFloor(PrevTilePoint, Quadrant, TileMap) && IsWall(CurTilePoint, Quadrant, TileMap))
			{
				FRow NextRow = Row.Next();
				NextRow.EndSlop = FFraction{ Col * 2 - 1, Row.Depth * 2 };
				Scan(NextRow, Quadrant, TileMap, Origin, MaxDepth);
			}
		}
		
		bHasPrev = true;
		PrevTilePoint = CurTilePoint;
	}
	
	if (IsFloor(PrevTilePoint, Quadrant, TileMap))
	{
		Scan(Row.Next(), Quadrant, TileMap, Origin, MaxDepth);
	}
}

bool AFOWManager::IsWall(FIntPoint& Tile, const FQuadrant& Quadrant, AFOWTileMap* TileMap) const
{
	const FIntPoint T = Quadrant.Transform(Tile.X, Tile.Y);
	const FTile* FTilePtr = TileMap->GetTile(T.X, T.Y);
	if (!FTilePtr) return false;
	return FTilePtr->Type == ETileType::Wall;
}

bool AFOWManager::IsFloor(FIntPoint& Tile, const FQuadrant& Quadrant, AFOWTileMap* TileMap) const
{
	const FIntPoint T = Quadrant.Transform(Tile.X, Tile.Y);
	const FTile* FTilePtr = TileMap->GetTile(T.X, T.Y);
	if (!FTilePtr) return false;
	return FTilePtr->Type == ETileType::Floor;
}

void AFOWManager::Reveal(FIntPoint& Tile, const FQuadrant& Quadrant, AFOWTileMap* TileMap, const FIntPoint& Origin, int32 MaxDepth)
{
	const FIntPoint T = Quadrant.Transform(Tile.X, Tile.Y);
	if (!TileMap->IsInMap(T.X, T.Y)) return;
	
	const int32 DX = T.X - Origin.X;
	const int32 DY = T.Y - Origin.Y;
	if (DX * DX + DY * DY > MaxDepth * MaxDepth) return;
	
	TileMap->SetTileVisibility(T.X, T.Y, true);
}

bool AFOWManager::IsSymmetric(FRow& Row, int32 Depth, int32 Col) const
{
	// col >= depth * start_slope
	// col * Denominator >= depth * Numerator
	bool bAboveStart = Col * Row.StartSlop.Denominator >= Depth * Row.StartSlop.Numerator;
    
	// col <= depth * end_slope  
	bool bBelowEnd = Col * Row.EndSlop.Denominator <= Depth * Row.EndSlop.Numerator;
    
	return bAboveStart && bBelowEnd;
}

void AFOWManager::UpdateEnemyVisibility(AFOWTileMap* MyTileMap, TArray<TScriptInterface<ISightProvider>>& EnemyProviders)
{
	if (!MyTileMap)
	{
		return;
	}
	
	for (const TScriptInterface<ISightProvider>& Provider : EnemyProviders)
	{
		UObject* Obj = Provider.GetObject();
		
		if (!SightProviderHelper::IsHideable(Obj))
		{
			continue;
		}
		
		FVector Location = SightProviderHelper::GetSightOrigin(Obj);
		FIntPoint TilePoint = MyTileMap->WorldToTile(Location);
		bool bVisible = MyTileMap->IsVisibleTile(TilePoint.X, TilePoint.Y);
		
		SightProviderHelper::ApplyFOWVisibility(Obj, bVisible);
	}
}

void AFOWManager::UpdateEnemyVisibility_Server(AFOWTileMap* TeamTileMap,
	TArray<TScriptInterface<ISightProvider>>& EnemyProviders, ERiftSightTag TeamTag)
{
	if (!HasAuthority() || !TeamTileMap) return;

	for (const TScriptInterface<ISightProvider>& Provider : EnemyProviders)
	{
		UObject* Obj = Provider.GetObject();
		if (!SightProviderHelper::IsHideable(Obj))
		{
			continue;
		}
		
		FVector Location = SightProviderHelper::GetSightOrigin(Obj);
		FIntPoint TilePoint = TeamTileMap->WorldToTile(Location);
		bool bIsVisible = TeamTileMap->IsVisibleTile(TilePoint.X, TilePoint.Y);

		// 서버가 Replicated 변수에 기록 → OnRep으로 클라이언트 전파
		SightProviderHelper::SetFOWVisibilityFlag(Obj, TeamTag, bIsVisible);
	}
}

AFOWTileMap* AFOWManager::GetLocalTileMap() const
{
	if (LocalClientTeam == ERiftSightTag::Red)
	{
		return RedTileMap;
	}
	return BlueTileMap;
}

void AFOWManager::SetLocalClientTeam(ERiftSightTag InTeam)
{
	// 1) None 가드
	if (InTeam == ERiftSightTag::None) { return; }

	// 2) 중복 호출 가드: 같은 팀이고 이미 비주얼 준비됨
	if (LocalClientTeam == InTeam && IsLocalTileMapReady()) { return; }

	PRINTLOG_TK(TEXT("[FOWManager] SetLocalClientTeam %d → %d"),
		(int32)LocalClientTeam, (int32)InTeam);

	LocalClientTeam = InTeam;

	AFOWTileMap* LocalTM = GetLocalTileMap();
	if (!LocalTM || !FOWVolume) { return; }

	// 3) 클라이언트는 자기 팀 타일 데이터를 여기서 생성
	//    (서버는 BeginPlay에서 이미 양 팀 모두 생성됨)
	//    GetTileSize() < 0.f면 아직 GenerateTileData 안 된 상태
	if (LocalTM->GetTileSize() < 0.f)
	{
		LocalTM->GenerateTileData(FOWVolume);
	}

	// 4) 비주얼 리소스 생성 (서버 호스트/클라이언트 공통 진입점)
	//    내부에 중복 호출 가드 있음
	LocalTM->CreateVisualResources();

	// 5) HUD에 텍스처 준비 완료 알림
	BroadcastFOWReadyIfValid();
	
	RefreshAllVisibility();
}

void AFOWManager::RefreshAllVisibility()
{
	for (const auto& Provider : RedSightProviders)
	{
		if (ALoLCharacterBase* Char = Cast<ALoLCharacterBase>(Provider.GetObject()))
		{
			Char->OnRep_FOWVisibility();  // 강제 재평가
		}
	}
	for (const auto& Provider : BlueSightProviders)
	{
		if (ALoLCharacterBase* Char = Cast<ALoLCharacterBase>(Provider.GetObject()))
		{
			Char->OnRep_FOWVisibility();
		}
	}
}




















