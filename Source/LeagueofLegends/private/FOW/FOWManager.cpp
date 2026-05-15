// Fill out your copyright notice in the Description page of Project Settings.


#include "FOW/FOWManager.h"

#include "LeagueofLegends.h"
#include "Interfaces/SightProviderHelper.h"
#include "FOW/FOWTileMap.h"
#include "GameFramework/RiftGameState.h"
#include "GameFramework/RiftPlayerState.h"
#include "Kismet/GameplayStatics.h"

#define TEST 0

// Sets default values
AFOWManager::AFOWManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AFOWManager::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (auto* GS = GetWorld()->GetGameState<ARiftGameState>())
	{
		GS->SetFOWManager(this);
	}
}

// Called when the game starts or when spawned
void AFOWManager::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Warning, TEXT("[FOWManager] LocalClientTeam=%d (0=None, 1=Blue, 2=Red)"),
		(int32)LocalClientTeam);

	// TODO: LocalPlayer의 팀을 가져와 세팅
	if (APlayerController* LocalPC = GetWorld()->GetFirstPlayerController())
	{
		if (ARiftPlayerState* PS = LocalPC->GetPlayerState<ARiftPlayerState>())
		{
			if (PS->GetTeam() != ETeam::None)
			{
				LocalClientTeam = (PS->GetTeam() == ETeam::Blue)
					? ERiftSightTag::Blue : ERiftSightTag::Red;
			}
		}
	}
	
	if (FOWVolume)
	{
		if (HasAuthority()) // 서버는 양쪽 타일맵 모두 생성
		{
			// 호스트 팀 → 렌더링 리소스 포함, 상대 팀 → 타일 데이터만
			if (LocalClientTeam == ERiftSightTag::Red)
			{
				RedTileMap->Generate(FOWVolume, false);  // 비주얼 O
				BlueTileMap->Generate(FOWVolume, true);   // 타일만
			}
			else
			{
				RedTileMap->Generate(FOWVolume, true);
				BlueTileMap->Generate(FOWVolume, false);
			}
		}
		else
		{
			if (LocalClientTeam == ERiftSightTag::Red)
			{
				RedTileMap->Generate(FOWVolume);
			}
			else
			{
				BlueTileMap->Generate(FOWVolume);
			}
		}
	}
	
#if TEST
	// 현재 playerPawn을 TestActor로 할당
	TestActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
#endif
	
	if (AFOWTileMap* LocalTileMap = GetLocalTileMap())
	{
		if (UTexture2D* Tex = LocalTileMap->GetFogTexture())
		{
			OnFOWReady.Broadcast(Tex);
		}
	}
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
		
		// 서버 권한 가시성 판정 (Server에서만 판정, Client는 서버 결과 받아서 적용)
		// Blue 적들이 Red 시야에 보이는가?
		UpdateEnemyVisibility_Server(RedTileMap, BlueSightProviders, ERiftSightTag::Red);
		// Red 적들이 Blue 시야에 보이는가?
		UpdateEnemyVisibility_Server(BlueTileMap, RedSightProviders, ERiftSightTag::Blue);
		
		// 리슨 서버 호스트: 자기 팀 텍스처만 갱신
		if (LocalClientTeam == ERiftSightTag::Red)
		{
			RedTileMap->UpdateFogTexture();
		}
		else
		{
			BlueTileMap->UpdateFogTexture();
		}
	}
	else
	{
		if (LocalClientTeam == ERiftSightTag::Red)
		{
			UpdateFOV(RedTileMap, RedSightProviders, true);
		}
		else
		{
			UpdateFOV(BlueTileMap, BlueSightProviders, true);
		}
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

	if (SightProviderHelper::GetTeam(SightObject) == ERiftSightTag::Red)
	{
		// SightObject 이름과, GetTeam 결과를 로그에 출력
		// PRINTLOG_TK(TEXT("Registering SightProvider: %s, Team=%s"), *SightObject->GetName(), TEXT("Red"));
		RedSightProviders.Add(SightProvider);
	}
	else
	{
		// PRINTLOG_TK(TEXT("Registering SightProvider: %s, Team=%s"), *SightObject->GetName(), TEXT("Blue"));
		BlueSightProviders.Add(SightProvider);
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

	if (SightProviderHelper::GetTeam(SightObject) == ERiftSightTag::Red)
	{
		RedSightProviders.Remove(SightProvider);
	}
	else
	{
		BlueSightProviders.Remove(SightProvider);
	}
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
	if (LocalClientTeam == InTeam) { return; }

	UE_LOG(LogTemp, Warning, TEXT("[FOWManager] SetLocalClientTeam %d → %d"), (int32)LocalClientTeam, (int32)InTeam);
	LocalClientTeam = InTeam;

	// 클라이언트: 이미 잘못된 팀으로 TileMap이 생성됐으므로 올바른 팀으로 재생성
	if (!HasAuthority() && FOWVolume)
	{
		if (LocalClientTeam == ERiftSightTag::Red)
		{
			RedTileMap->Generate(FOWVolume);
		}
		
		else
		{
			BlueTileMap->Generate(FOWVolume);
		}
		
		if (AFOWTileMap* LocalTileMap = GetLocalTileMap())
		{
			if (UTexture2D* Tex = LocalTileMap->GetFogTexture())
			{
				OnFOWReady.Broadcast(Tex);
			}
		}
	}
}




















