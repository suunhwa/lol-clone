// Fill out your copyright notice in the Description page of Project Settings.


#include "FOW/FOWTileMap.h"


AFOWTileMap::AFOWTileMap()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFOWTileMap::BeginPlay()
{
	Super::BeginPlay();
}

void AFOWTileMap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFOWTileMap::GenerateFromMap(AActor* MapActor)
{
	// Map의 BoundingBox를 가져와서 TileMap의 크기를 결정
	FBox Bounds = MapActor->GetComponentsBoundingBox();
	FVector WorldMin = Bounds.Min;
	FVector WorldMax = Bounds.Max;

	float MapWidthLength = WorldMax.X - WorldMin.X;
	float MapHeightLength = WorldMax.Y - WorldMin.Y;

	TileSize = FMath::Max(MapWidthLength, MapHeightLength) / MapSize;
	float HalfTileSize = TileSize / 2.f;
	float RayHeight = WorldMax.Z + 100.f; // Ray의 시작 높이

	for (int i = 0; i < MapSize; i++)
	{
		for (int j = 0; j < MapSize; j++)
		{
			float TileCenterX = WorldMin.X + i * TileSize + HalfTileSize;
			float TileCenterY = WorldMin.Y + j * TileSize + HalfTileSize;

			FVector RayStart(TileCenterX, TileCenterY, RayHeight);
			FVector RayEnd(TileCenterX, TileCenterY, WorldMin.Z - 100.f);

			FHitResult HitResult;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this); // 자신은 무시

			bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, RayStart, RayEnd, ECC_Visibility, Params);

			if (bHit)
			{
				// 타일이 지형과 충돌한 경우
				// HitResult.Location을 사용하여 타일의 높이를 결정할 수 있음
				// 예: SetTileHeight(i, j, HitResult.Location.Z);
				// DebugBox 그리기
				FVector BoxCenter(TileCenterX, TileCenterY, HitResult.Location.Z);
				FVector BoxExtent(HalfTileSize, HalfTileSize, 10.f);

				FColor BoxColor = BoxCenter.Z < 150.f ? FColor::Green : FColor::Red;
				DrawDebugBox(GetWorld(), BoxCenter, BoxExtent, BoxColor, true, -1.f);
			}
			else
			{
				// 타일이 지형과 충돌하지 않은 경우
				// 예: SetTileHeight(i, j, DefaultHeight);
			}
		}
	}
}

FTile* AFOWTileMap::GetTile(int32 X, int32 Y)
{
	if (!IsValidTile(X, Y))
	{
		return nullptr;
	}

	const int32 Index = Y * MapSize + X;
	return &Tiles[Index];
}

const FTile* AFOWTileMap::GetTile(int32 X, int32 Y) const
{
	if (!IsValidTile(X, Y))
	{
		return nullptr;
	}

	const int32 Index = Y * MapSize + X;
	return &Tiles[Index];
}

bool AFOWTileMap::IsValidTile(int32 X, int32 Y) const
{
	return X >= 0 && X < MapSize && Y >= 0 && Y < MapSize;
}

bool AFOWTileMap::IsInMap(int32 X, int32 Y) const
{
	return IsValidTile(X, Y) && GetTile(X, Y)->IsInMap(); // IsValid가 false면 뒤에 검사 안함
}

bool AFOWTileMap::IsVisibleTile(int32 X, int32 Y) const
{
	if (!IsInMap(X, Y))
	{
		UE_LOG(LogTemp, Warning, TEXT("IsVisibleTile: Invalid tile coordinates (%d, %d)"), X, Y);
		return false;
	}
	return GetTile(X, Y)->bIsVisible;
}

void AFOWTileMap::SetTileVisibility(int32 X, int32 Y, bool bVisible)
{
	if (!IsInMap(X, Y))
	{
		UE_LOG(LogTemp, Warning, TEXT("SetTileVisibility: Invalid tile coordinates (%d, %d)"), X, Y);
		return;
	}
	GetTile(X, Y)->bIsVisible = bVisible;
}
