// Fill out your copyright notice in the Description page of Project Settings.


#include "FOW/FOWTileMap.h"

#include "FOW/FOWVolume.h"
#include "LeagueofLegends.h"
#include "Engine/StaticMeshActor.h"
#include "Interfaces/SightProviderHelper.h"
#include "Kismet/GameplayStatics.h"

AFOWTileMap::AFOWTileMap()
{
	PrimaryActorTick.bCanEverTick = true;
	Tiles.SetNum(MapSize * MapSize); // 타일 배열 초기화
}

AFOWTileMap::~AFOWTileMap() = default;

void AFOWTileMap::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AFOWTileMap::BeginPlay()
{
	Super::BeginPlay();

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
	
	if (SightDataBuffer)
	{
		delete[] SightDataBuffer;
		SightDataBuffer = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AFOWTileMap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// if (FOWPostProcessMID)
	// {
	// 	FVector PlayerWorldPos = UGameplayStatics::GetPlayerPawn(GetWorld(), 0)->GetActorLocation();
	// 	FOWPostProcessMID->SetVectorParameterValue(
	// 		TEXT("PlayerLocation"),
	// 		FLinearColor(PlayerWorldPos.X, PlayerWorldPos.Y, 0, 0));
	// }
}

void AFOWTileMap::GenerateTileData(AFOWVolume* FOWVolume)
{
	// 중복 호출 가드: 이미 생성됐으면 스킵
	if (TileSize > 0.f)
	{
		return;
	}
    
	if (Tiles.Num() != MapSize * MapSize)
	{
		Tiles.SetNum(MapSize * MapSize);
	}
	
	GenerateTileMap(FOWVolume);
}

void AFOWTileMap::CreateVisualResources()
{
	if (FogTexture)
	{
		return;
	}

	CreateFogTexture();
	CreateDebugPlane();
	UpdateFogTexture();
	CreateFOWPostProcess();
}

void AFOWTileMap::GenerateTileMap(AFOWVolume* FOWVolume)
{
	if (!FOWVolume)
	{
		PRINTLOG_TK(TEXT("GenerateFromMap: MapActor is null"));
		return;
	}

	// Map의 BoundingBox를 가져와서 TileMap의 크기를 결정
	FBox Bounds = FOWVolume->GetFOWBounds();
	WorldMin = Bounds.Min;
	FVector WorldMax = Bounds.Max;

	float MapXLength = WorldMax.X - WorldMin.X;
	float MapYLength = WorldMax.Y - WorldMin.Y;

	TileSize = FMath::Max(MapXLength, MapYLength) / MapSize;
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
				float Diff = 8600.f - HitResult.Location.Z;
				bool bIsFloor = Diff > -45.f;
				bool bIsSightProvider = SightProviderHelper::IsSightProvider(HitResult.GetActor());
				UPrimitiveComponent* HitComp = HitResult.GetComponent();
				bool bIsNonSightBlock = HitComp && HitComp->GetCollisionProfileName() == FName("NonSightBlock");
				
				bool bIsFloorForDebug;
				
				if (bIsFloor || bIsSightProvider || bIsNonSightBlock)
				{
					CurTile.Type = ETileType::Floor; // 예시로 Floor 타입으로 설정
					bIsFloorForDebug = true;
				}
				else
				{
					CurTile.Type = ETileType::Wall; // 예시로 Wall 타입으로 설정
					bIsFloorForDebug = false;
				}

				// DebugBox 그리기
				if (bDrawTileBox)
				{
					FVector BoxCenter(TileCenterX, TileCenterY, HitResult.Location.Z);
					FVector BoxExtent(HalfTileSize, HalfTileSize, 10.f);
					
					FColor BoxColor = bIsFloorForDebug ? FColor::Green : FColor::Red;
					DrawDebugBox(GetWorld(), BoxCenter, BoxExtent, BoxColor, true, -1.f);
				}
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
		PRINTLOG_TK(TEXT("SetTile: Invalid tile coordinates (%d, %d)"), X, Y);
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
		PRINTLOG_TK(TEXT("IsVisibleTile: Invalid tile coordinates (%d, %d)"), X, Y);
		return false;
	}
	return GetTile(X, Y)->bIsVisible;
}

void AFOWTileMap::SetTileVisibility(int32 X, int32 Y, bool bVisible)
{
	if (!IsInMap(X, Y))
	{
		PRINTLOG_TK(TEXT("SetTileVisibility: Invalid tile coordinates (%d, %d)"), X, Y);
		return;
	}
	GetTile(X, Y)->bIsVisible = bVisible;
}

void AFOWTileMap::CreateFogTexture()
 {
 	if (!FogTexture)
 	{
 		int32 TextureSize;
 		
 		if (bUseUpscaler)
 		{
 			Upscaler = MakeUnique<FFOWUpscaler>(MapSize);
 			TextureSize = Upscaler->GetUpscaledSize();
 		}
 		else
 		{
 			TextureSize = MapSize;
 		}
 		
 		FogTexture = UTexture2D::CreateTransient(TextureSize, TextureSize, PF_G8);
 		if (!FogTexture)
 		{
 			PRINTLOG_TK(TEXT("CreateDebugTexture: Failed to create transient texture"));
 			return;
 		}
 		
 		FogTexture->Filter = TF_Bilinear; // 부드러운 경계
 		FogTexture->CompressionSettings = TC_Grayscale;
 		FogTexture->AddressX = TA_Clamp;
 		FogTexture->AddressY = TA_Clamp;
 		FogTexture->SRGB = false;
 		FogTexture->UpdateResource();
 	}
 
 	// uint8 버퍼 동적 할당
 	PixelBufferSize = MapSize * MapSize * sizeof(uint8);
 	if (!PixelBuffer)
 	{
 		PixelBuffer = new uint8[MapSize * MapSize];
 	}
 	FMemory::Memzero(PixelBuffer, PixelBufferSize);// 전부 검정으로 초6기화
 }

void AFOWTileMap::CreateSightDataTexture()
{	
	SightDataTexture = UTexture2D::CreateTransient(MaxSightProviders, 1, PF_A32B32G32R32F);
	SightDataTexture->Filter = TF_Nearest;
	SightDataTexture->SRGB = false;
	SightDataTexture->UpdateResource();

	SightDataBuffer = new FLinearColor[MaxSightProviders];
	FMemory::Memzero(SightDataBuffer, MaxSightProviders * sizeof(FLinearColor));
}

void AFOWTileMap::CreateFOWPostProcess()
{
	if (!FOWPostProcessMaterial)
	{
		PRINTLOG_TK(TEXT("CreateFOWPostProcess: FOWPostProcessMaterial is null"));
		return;
	}

	if (!FOWPostProcessMID)
	{
		FOWPostProcessMID = UMaterialInstanceDynamic::Create(FOWPostProcessMaterial, this);
	}

	if (!FOWPostProcessMID)
	{
		PRINTLOG_TK(TEXT("CreateFOWPostProcess: Failed to create dynamic material instance"));
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

	// FogTexture
	FOWPostProcessMID->SetTextureParameterValue(
		TEXT("FogTexture"), FogTexture
	);
	
	// SightDataTexture
	FOWPostProcessMID->SetTextureParameterValue(
		TEXT("SightDataTexture"), SightDataTexture
	);

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

void AFOWTileMap::CreateDebugPlane()
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
			PRINTLOG_TK(TEXT("CreateDebugTexture: Failed to create dynamic material instance"));
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
		PRINTLOG_TK(TEXT("UpdateDebugTexture: DebugTexture or PixelBuffer is null"));
		return;
	}

	for (int32 Y = 0; Y < MapSize; Y++)
	{
		for (int32 X = 0; X < MapSize; X++)
		{
			const FTile* Tile = GetTile(X, Y);

			// Wall이면 255(흰색), Floor면 0(검정)
			PixelBuffer[Y * MapSize + X] = (Tile && Tile->bIsVisible) ? 255 : 0;
		}
	}
	
	uint8* SourceData;
	int32 TextureSize;

	if (bUseUpscaler && Upscaler)
	{
		// Marching Squares 업스케일: 128x128 → 512x512
		Upscaler->Upscale(this);
		SourceData = Upscaler->GetBuffer();
		TextureSize = Upscaler->GetUpscaledSize();
	}
	else
	{
		SourceData = PixelBuffer;
		TextureSize = MapSize;
	}

	// UpdateTextureRegions: LockTexture2D 방식보다 안전하고
	// UpdateResource() 타이밍 이슈 없이 렌더 스레드에 올바르게 전달됨
	// SrcPitch = MapSize * 1byte(PF_G8), SrcBpp = 1byte
	FUpdateTextureRegion2D* Region = new FUpdateTextureRegion2D(
		0, 0, 0, 0, TextureSize, TextureSize);
	
	FogTexture->UpdateTextureRegions(
		0,
		1,
		Region,
		static_cast<uint32>(TextureSize),
		sizeof(uint8),
		SourceData,
		[](uint8* /*SrcData*/, const FUpdateTextureRegion2D* InRegion)
		{
			delete InRegion; // Region 힙 해제
		}
	);
}

void AFOWTileMap::UpdateSightDataTexture(const TArray<TScriptInterface<ISightProvider>>& SightProviders)
{
	if (!SightDataTexture || !SightDataBuffer) return;

	const int32 ActiveCount = SightProviders.Num();
	
	FMemory::Memzero(SightDataBuffer, MaxSightProviders * sizeof(FLinearColor));

	// 활성 슬롯 채우기
	for (int32 i = 0; i < ActiveCount; i++)
	{
		UObject* Obj = SightProviders[i].GetObject();
		FVector Origin = SightProviderHelper::GetSightOrigin(Obj);
		float Range = SightProviderHelper::GetSightRange(Obj);

		SightDataBuffer[i] = FLinearColor(Origin.X, Origin.Y, Range, 1.f);
		
		// 디버깅
		// PRINTLOG_TK(TEXT("SightProvider %d: Origin=(%.1f, %.1f), Range=%.1f"), i, Origin.X, Origin.Y, Range);
	}
	
	// PRINTLOG_TK(TEXT("Updating SightDataTexture with %d active sight providers"), ActiveCount);

	// FUpdateTextureRegion2D* Region = new FUpdateTextureRegion2D(
	// 	0, 0, 0, 0, ActiveCount, 1);

	FUpdateTextureRegion2D* Region = new FUpdateTextureRegion2D(
		0, 0, 0, 0, MaxSightProviders, 1);
	
	SightDataTexture->UpdateTextureRegions(
		0,
		1,
		Region,
		MaxSightProviders * sizeof(FLinearColor),
		sizeof(FLinearColor),
		(uint8*)SightDataBuffer,
		[](uint8*, const FUpdateTextureRegion2D* R) { delete R; }
	);
	
	if (FOWPostProcessMID)
	{
		FOWPostProcessMID->SetScalarParameterValue(TEXT("SightCount"), ActiveCount);
	}
}

void AFOWTileMap::ResetTileVisibility()
{
	for (FTile& Tile : Tiles)
	{
		Tile.bIsVisible = false;
	}
}
