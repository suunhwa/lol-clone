// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TileData.h"
#include "FOW/FOWUpscaler.h"
#include "FOWTileMap.generated.h"

class ISightProvider;
class AFOWVolume;
class FFOWUpscaler;

UCLASS()
class LEAGUEOFLEGENDS_API AFOWTileMap : public AActor
{
	GENERATED_BODY()

public:
	AFOWTileMap();
	virtual ~AFOWTileMap();

	virtual void PostInitializeComponents() override;
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Tick(float DeltaTime) override;
	
public:
	UFUNCTION(BlueprintCallable)
	void Generate(AFOWVolume* FOWVolume);
	
	UFUNCTION(BlueprintCallable)
	void GenerateTileMap(AFOWVolume* FOWVolume);
	
	UFUNCTION(BlueprintCallable)
	void UpdateFogTexture();
	
	UFUNCTION(BlueprintCallable)
	void UpdateSightDataTexture(const TArray<TScriptInterface<ISightProvider>>& SightProviders);
	
	UFUNCTION(BlueprintCallable)
	void ResetTileVisibility();
	
	FIntPoint WorldToTile(const FVector& WorldLocation) const;
	FVector2D TileToUV(const FIntPoint& Tile) const;

#pragma region Getter Setter
	FTile* GetTile(int32 X, int32 Y);
	const FTile* GetTile(int32 X, int32 Y) const;
	void SetTile(int32 X, int32 Y, const FTile& NewTile);
	float GetTileSize() const { return TileSize; }
	float GetVolumeExtentXY() const
	{
		return TileSize * MapSize / 2.f;
	}
#pragma endregion
	
#pragma region Utility
	bool IsValidRange(int32 X, int32 Y) const;
	bool IsInMap(int32 X, int32 Y) const;
	bool IsVisibleTile(int32 X, int32 Y) const;
	void SetTileVisibility(int32 X, int32 Y, bool bVisible);
#pragma endregion 
	
private:
	void CreateFogTexture();
	void CreateSightDataTexture();
	void CreateFOWPostProcess();
	
	void CreateDebugPlane();
	
public:
	static constexpr int32 MapSize = 128;
	
protected:
	UPROPERTY()
	float TileSize = -1.f; // 음수면 유효 하지 않음.

	UPROPERTY()
	FVector WorldMin;
	
	UPROPERTY()
	TArray<FTile> Tiles;
	
#pragma region Texture (Create in runtime)
	UPROPERTY()
	TObjectPtr<UTexture2D> FogTexture;
	
	UPROPERTY()
	TObjectPtr<UTexture2D> SightDataTexture;
	
	static constexpr int32 MaxSightProviders = 128;
#pragma endregion
	
#pragma region PostProcess
	UPROPERTY(EditAnywhere)
	TObjectPtr<UMaterialInterface> FOWPostProcessMaterial;
	
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> FOWPostProcessMID;
#pragma endregion
	
#pragma region Debug
	UPROPERTY(EditAnywhere)
	TObjectPtr<AStaticMeshActor> DebugPlane; // 에디터에서 Plane 연결

	UPROPERTY(EditAnywhere)
	TObjectPtr<UMaterialInterface> DebugMaterial; // 에디터에서 M_TileMapDebug 연결

	UPROPERTY()
	UMaterialInstanceDynamic* DebugMID;
#pragma endregion
	
	uint8* PixelBuffer = nullptr;
	uint32 PixelBufferSize = 0;
	
	FLinearColor* SightDataBuffer = nullptr;
	
	UPROPERTY(EditAnywhere, Category = "FOW|Debug")
	bool bDrawTileBox = false;
	
	UPROPERTY(EditAnywhere, Category = "FOW|Debug")
	bool bUseUpscaler = true;
	
	TUniquePtr<FFOWUpscaler> Upscaler;
};
