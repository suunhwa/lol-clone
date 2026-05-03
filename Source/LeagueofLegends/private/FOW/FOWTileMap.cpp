// Fill out your copyright notice in the Description page of Project Settings.


#include "FOW/FOWTileMap.h"

#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"

AFOWTileMap::AFOWTileMap()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFOWTileMap::BeginPlay()
{
	Super::BeginPlay();

	Tiles.SetNum(MapSize * MapSize); // 타일 배열 초기화
	// CreateDebugTexture();
}

void AFOWTileMap::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (PixelBuffer)
	{
		delete[] PixelBuffer;
		PixelBuffer = nullptr;
		PixelBufferSize = 0;
	}

	Super::EndPlay(EndPlayReason);
}

void AFOWTileMap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (FOWPostProcessMID)
	{
		FVector PlayerWorldPos = UGameplayStatics::GetPlayerPawn(GetWorld(), 0)->GetActorLocation();
		FOWPostProcessMID->SetVectorParameterValue(
			TEXT("PlayerLocation"),
			FLinearColor(PlayerWorldPos.X, PlayerWorldPos.Y, 0, 0));
	}
}

void AFOWTileMap::Generate(AActor* MapActor)
{
	GenerateTileMap(MapActor);
	CreateFogTexture();
	
	// Debug용 Plane에 텍스처 연결
	SetDebugPlane();
	
	UpdateFogTexture();
	CreateFOWPostProcess();
}

void AFOWTileMap::GenerateTileMap(AActor* MapActor)
{
	if (!MapActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("GenerateFromMap: MapActor is null"));
		return;
	}

	// Map의 BoundingBox를 가져와서 TileMap의 크기를 결정
	FBox Bounds = MapActor->GetComponentsBoundingBox();
	WorldMin = Bounds.Min;
	FVector WorldMax = Bounds.Max;

	float MapWidthLength = WorldMax.X - WorldMin.X;
	float MapHeightLength = WorldMax.Y - WorldMin.Y;

	TileSize = FMath::Max(MapWidthLength, MapHeightLength) / MapSize;
	float HalfTileSize = TileSize / 2.f;
	float RayHeight = WorldMax.Z + 1000.f; // Ray의 시작 높이

	for (int i = 0; i < MapSize; i++)
	{
		for (int j = 0; j < MapSize; j++)
		{
			float TileCenterX = WorldMin.X + j * TileSize + HalfTileSize;
			float TileCenterY = WorldMin.Y + i * TileSize + HalfTileSize;

			FVector RayStart(TileCenterX, TileCenterY, RayHeight);
			FVector RayEnd(TileCenterX, TileCenterY, WorldMin.Z - 1000.f);

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

				// // DebugBox 그리기
				// FVector BoxCenter(TileCenterX, TileCenterY, HitResult.Location.Z);
				// FVector BoxExtent(HalfTileSize, HalfTileSize, 10.f);
				//
				// FColor BoxColor = BoxCenter.Z < 150.f ? FColor::Green : FColor::Red;
				// DrawDebugBox(GetWorld(), BoxCenter, BoxExtent, BoxColor, true, -1.f);
			}
			else
			{
				// 타일이 지형과 충돌하지 않은 경우
				CurTile.Type = ETileType::None;
				// 예: SetTileHeight(i, j, DefaultHeight);
			}
		}
	}

	// CreateFogTexture();
	// UpdateDebugTexture();
	// CreateFOWPostProcess();
}

FIntPoint AFOWTileMap::WorldToTile(const FVector& WorldLocation) const
{
	int32 TileX = FMath::FloorToInt((WorldLocation.X - WorldMin.X) / TileSize);
	int32 TileY = FMath::FloorToInt((WorldLocation.Y - WorldMin.Y) / TileSize);
	return FIntPoint(TileX, TileY);
}

FVector2D AFOWTileMap::TileToUV(const FIntPoint& Tile) const
{
	if (!IsValidRange(Tile.X, Tile.Y))
	{
		return FVector2D::ZeroVector;
	}

	float U = static_cast<float>(Tile.X) / MapSize;
	float V = static_cast<float>(Tile.Y) / MapSize;
	return FVector2D(U, V);
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

void AFOWTileMap::CreateFogTexture()
{
	if (!FogTexture)
	{
		FogTexture = UTexture2D::CreateTransient(MapSize, MapSize, PF_G8);
		if (!FogTexture)
		{
			UE_LOG(LogTemp, Warning, TEXT("CreateDebugTexture: Failed to create transient texture"));
			return;
		}

		FogTexture->Filter = TF_Nearest; // 픽셀 경계 선명하게
		FogTexture->CompressionSettings = TC_Grayscale;
		FogTexture->AddressX = TA_Clamp;
		FogTexture->AddressY = TA_Clamp;
		FogTexture->SRGB = false;
		FogTexture->UpdateResource();
		FlushRenderingCommands();
	}

	// uint8 버퍼 동적 할당
	PixelBufferSize = MapSize * MapSize * sizeof(uint8);
	if (!PixelBuffer)
	{
		PixelBuffer = new uint8[MapSize * MapSize];
	}
	FMemory::Memset(PixelBuffer, 0, PixelBufferSize); // 전부 검정으로 초기화
}

void AFOWTileMap::CreateFOWPostProcess()
{
	if (!FOWPostProcessMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateFOWPostProcess: FOWPostProcessMaterial is null"));
		return;
	}

	if (!FOWPostProcessMID)
	{
		FOWPostProcessMID = UMaterialInstanceDynamic::Create(FOWPostProcessMaterial, this);
	}

	if (!FOWPostProcessMID)
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateFOWPostProcess: Failed to create dynamic material instance"));
		return;
	}

	// VolumeExtentXY
	FOWPostProcessMID->SetScalarParameterValue(
		TEXT("VolumeExtentXY"), GetVolumeExtentXY());

	// TileMapLocation
	FVector MapCenter = WorldMin + FVector(
		(TileSize * MapSize) / 2.f,
		(TileSize * MapSize) / 2.f,
		0.f
	);
	FOWPostProcessMID->SetVectorParameterValue(
		TEXT("MapCenter"), FLinearColor(
			MapCenter.X,
			MapCenter.Y,
			0, 0)
	);

	// FOWTexture
	FOWPostProcessMID->SetTextureParameterValue(
		TEXT("FogTexture"), FogTexture
	);

	// FOWPostProcessMID->SetScalarParameterValue(
	// 	TEXT("SightRadius"),
	// 	10
	// );

	// PostProcessVolume 찾아서 MID 적용
	APostProcessVolume* PPV = Cast<APostProcessVolume>(
		UGameplayStatics::GetActorOfClass(GetWorld(), APostProcessVolume::StaticClass())
	);

	if (PPV)
	{
		FWeightedBlendable Blendable;
		Blendable.Object = FOWPostProcessMID;
		Blendable.Weight = 1.0f;
		PPV->Settings.WeightedBlendables.Array.Add(Blendable);
	}
}

void AFOWTileMap::SetDebugPlane()
{
	// MID 생성 후 텍스처 연결
	if (DebugPlane && DebugMaterial)
	{
		if (!DebugMID)
		{
			DebugMID = UMaterialInstanceDynamic::Create(DebugMaterial, this);
		}

		if (!DebugMID)
		{
			UE_LOG(LogTemp, Warning, TEXT("CreateDebugTexture: Failed to create dynamic material instance"));
			return;
		}

		DebugMID->SetTextureParameterValue(TEXT("DebugTex"), FogTexture);
		DebugPlane->GetStaticMeshComponent()->SetMaterial(0, DebugMID);
	}
}

void AFOWTileMap::UpdateFogTexture()
{
	if (!FogTexture || !PixelBuffer)
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateDebugTexture: DebugTexture or PixelBuffer is null"));
		return;
	}

	for (int32 Y = 0; Y < MapSize; Y++)
	{
		for (int32 X = 0; X < MapSize; X++)
		{
			const FTile* Tile = GetTile(X, Y);

			// Wall이면 255(흰색), Floor면 0(검정)
			// PF_G8 포맷: 1바이트(uint8)가 머티리얼 샘플링 시 R채널로 출력됨
			// PixelBuffer[Y * MapSize + X] = (Tile && Tile->Type == ETileType::Wall) ? 255 : 0;
			PixelBuffer[Y * MapSize + X] = (Tile && Tile->bIsVisible) ? 255 : 0;
		}
	}

	// UpdateTextureRegions: LockTexture2D 방식보다 안전하고
	// UpdateResource() 타이밍 이슈 없이 렌더 스레드에 올바르게 전달됨
	// SrcPitch = MapSize * 1byte(PF_G8), SrcBpp = 1byte
	FUpdateTextureRegion2D* Region = new FUpdateTextureRegion2D(0, 0, 0, 0, MapSize, MapSize);
	FogTexture->UpdateTextureRegions(
		0, // MipIndex
		1, // NumRegions
		Region, // Regions
		static_cast<uint32>(MapSize), // SrcPitch (행당 바이트 수: MapSize * 1)
		sizeof(uint8), // SrcBpp (픽셀당 바이트 수: 1)
		PixelBuffer, // SrcData
		[](uint8* /*SrcData*/, const FUpdateTextureRegion2D* InRegion)
		{
			delete InRegion; // Region 힙 해제
		}
	);
}

void AFOWTileMap::ResetTileVisibility()
{
	for (FTile& Tile : Tiles)
	{
		Tile.bIsVisible = false;
	}
}
