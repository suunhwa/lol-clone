// Fill out your copyright notice in the Description page of Project Settings.


#include "FOW/FOWTileMap.h"

#include "Engine/StaticMeshActor.h"


AFOWTileMap::AFOWTileMap()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFOWTileMap::BeginPlay()
{
	Super::BeginPlay();

	Tiles.SetNum(MapSize * MapSize); // 타일 배열 초기화
	CreateDebugTexture();
}

void AFOWTileMap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFOWTileMap::GenerateFromMap(AActor* MapActor)
{
	// Map의 BoundingBox를 가져와서 TileMap의 크기를 결정
	FBox Bounds = MapActor->GetComponentsBoundingBox();
	WorldMin = Bounds.Min;
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
			float TileCenterX = WorldMin.X + j * TileSize + HalfTileSize;
			float TileCenterY = WorldMin.Y + i * TileSize + HalfTileSize;

			FVector RayStart(TileCenterX, TileCenterY, RayHeight);
			FVector RayEnd(TileCenterX, TileCenterY, WorldMin.Z - 100.f);

			FHitResult HitResult;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this); // 자신은 무시

			bool bHit = GetWorld()->LineTraceSingleByChannel(
				HitResult,
				RayStart,
				RayEnd,
				ECC_Visibility,
				Params
			);

			FTile& CurTile = Tiles[i * MapSize + j]; // 타일 배열에서 현재 타일 참조
			if (bHit)
			{
				// 타일이 지형과 충돌한 경우
				// HitResult.Location을 사용하여 타일의 높이를 결정할 수 있음
				// 예: SetTileHeight(i, j, HitResult.Location.Z);
				if (HitResult.Location.Z < 150.f)
				{
					CurTile.Type = ETileType::Floor; // 예시로 Floor 타입으로 설정
				}
				else
				{
					CurTile.Type = ETileType::Wall; // 예시로 Wall 타입으로 설정
				}

				// DebugBox 그리기
				FVector BoxCenter(TileCenterX, TileCenterY, HitResult.Location.Z);
				FVector BoxExtent(HalfTileSize, HalfTileSize, 10.f);

				FColor BoxColor = BoxCenter.Z < 150.f ? FColor::Green : FColor::Red;
				DrawDebugBox(GetWorld(), BoxCenter, BoxExtent, BoxColor, true, -1.f);
			}
			else
			{
				// 타일이 지형과 충돌하지 않은 경우
				CurTile.Type = ETileType::None;
				// 예: SetTileHeight(i, j, DefaultHeight);
			}
		}
	}
	
	// CreateDebugTexture();
	UpdateDebugTexture();
}

FTile* AFOWTileMap::GetTile(int32 X, int32 Y)
{
	if (!IsValidRange(X, Y))
	{
		return nullptr;
	}

	const int32 Index = Y * MapSize + X;
	return &Tiles[Index];
}

const FTile* AFOWTileMap::GetTile(int32 X, int32 Y) const
{
	if (!IsValidRange(X, Y))
	{
		return nullptr;
	}

	const int32 Index = Y * MapSize + X;
	return &Tiles[Index];
}

void AFOWTileMap::SetTile(int32 X, int32 Y, const FTile& NewTile)
{
	if (!IsValidRange(X, Y))
	{
		UE_LOG(LogTemp, Warning, TEXT("SetTile: Invalid tile coordinates (%d, %d)"), X, Y);
		return;
	}
	Tiles[Y * MapSize + X] = NewTile;
}

bool AFOWTileMap::IsValidRange(int32 X, int32 Y) const
{
	return X >= 0 && X < MapSize && Y >= 0 && Y < MapSize;
}

bool AFOWTileMap::IsInMap(int32 X, int32 Y) const
{
	return IsValidRange(X, Y) && GetTile(X, Y)->IsInMap(); // IsValid가 false면 뒤에 검사 안함
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

void AFOWTileMap::CreateDebugTexture()
{
	DebugTexture = UTexture2D::CreateTransient(MapSize, MapSize, PF_R8G8B8A8);
	DebugTexture->Filter = TF_Nearest; // 픽셀 경계 선명하게
	DebugTexture->UpdateResource();

	// 생성 직후 검정으로 초기화
	// TArray<FColor> PixelBuffer;
	// PixelBuffer.Init(FColor::Black, MapSize * MapSize);
	//
	// FTexture2DMipMap& Mip = DebugTexture->GetPlatformData()->Mips[0];
	// void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
	// FMemory::Memcpy(Data, PixelBuffer.GetData(), PixelBuffer.Num() * sizeof(FColor));
	// Mip.BulkData.Unlock();
	// DebugTexture->UpdateResource();
	
	// MID 생성 후 텍스처 연결
	if (DebugPlane && DebugMaterial)
	{
		DebugMID = UMaterialInstanceDynamic::Create(DebugMaterial, this);
		DebugMID->SetTextureParameterValue(TEXT("DebugTex"), DebugTexture);
		DebugPlane->GetStaticMeshComponent()->SetMaterial(0, DebugMID);
	}
}

void AFOWTileMap::UpdateDebugTexture()
{
	if (!DebugTexture)
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateDebugTexture: DebugTexture is null"));
		return;
	}

	// 픽셀 버퍼 생성
	TArray<FColor> PixelBuffer;
	PixelBuffer.SetNum(MapSize * MapSize);

	for (int32 Y = 0; Y < MapSize; Y++)
	{
		for (int32 X = 0; X < MapSize; X++)
		{
			const FTile& Tile = *GetTile(X, Y);

			// Wall이면 흰색, Floor면 검정
			PixelBuffer[Y * MapSize + X] = Tile.Type == ETileType::Wall
				? FColor::White
				: FColor::Black;
		}
	}

	// Texture2D에 픽셀 버퍼 업데이트
	FTexture2DMipMap& Mip = DebugTexture->GetPlatformData()->Mips[0];
	void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(Data, PixelBuffer.GetData(), PixelBuffer.Num() * sizeof(FColor));
	Mip.BulkData.Unlock();

	DebugTexture->UpdateResource();
}
