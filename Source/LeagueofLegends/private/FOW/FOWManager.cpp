// Fill out your copyright notice in the Description page of Project Settings.


#include "FOW/FOWManager.h"

#include "LeagueofLegends.h"
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

	// TODO: LocalPlayer의 팀을 가져와 세팅
	if (LocalClientTeam == ERiftSightTag::None)
	{
		LocalClientTeam = ERiftSightTag::Red; // 임시로 Red 팀으로 설정
	}
	
	if (MapActor)
	{
		if (LocalClientTeam == ERiftSightTag::Red)
		{
			RedTileMap->Generate(MapActor);
		}
		else
		{
			BlueTileMap->Generate(MapActor);
		}
	}
	
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
	if (LocalClientTeam == ERiftSightTag::Red)
	{
		UpdateFOV(RedTileMap, RedSightProviders);
	}
	else
	{
		UpdateFOV(BlueTileMap, BlueSightProviders);
	}
#endif
}

void AFOWManager::UpdateFOV(AFOWTileMap* TileMap, TArray<TScriptInterface<ISightProvider>>& SightProviders)
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

	TileMap->UpdateFogTexture();
#endif
}

void AFOWManager::RegisterSightProvider(UObject* SightObject)
{
	if (!SightObject || !SightProviderHelper::ImplementsSightProvider(SightObject))
	{
		PRINTLOG_TK(TEXT("RegisterSightProvider: Invalid SightObject"));
		return;
	}

	TScriptInterface<ISightProvider> SightProvider;
	SightProvider.SetObject(SightObject);
	SightProvider.SetInterface(SightProviderHelper::TryGetProvider(SightObject));

	if (SightProviderHelper::GetTeam(SightObject) == ERiftSightTag::Red)
	{
		RedSightProviders.Add(SightProvider);
	}
	else
	{
		BlueSightProviders.Add(SightProvider);
	}
}

void AFOWManager::UnregisterSightProvider(UObject* SightObject)
{
	if (!SightObject || !SightProviderHelper::ImplementsSightProvider(SightObject))
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
		Scan(FRow(), Quadrant, TileMap, MaxDepth);
	}
}

void AFOWManager::Scan(FRow Row, const FQuadrant& Quadrant, AFOWTileMap* TileMap, int32 MaxDepth)
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
			Reveal(CurTilePoint, Quadrant, TileMap);
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
				Scan(NextRow, Quadrant, TileMap, MaxDepth);
			}
		}
		
		bHasPrev = true;
		PrevTilePoint = CurTilePoint;
	}
	
	if (IsFloor(PrevTilePoint, Quadrant, TileMap))
	{
		Scan(Row.Next(), Quadrant, TileMap, MaxDepth);
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

void AFOWManager::Reveal(FIntPoint& Tile, const FQuadrant& Quadrant, AFOWTileMap* TileMap)
{
	const FIntPoint T = Quadrant.Transform(Tile.X, Tile.Y);
	if (!TileMap->IsInMap(T.X, T.Y)) return;
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

