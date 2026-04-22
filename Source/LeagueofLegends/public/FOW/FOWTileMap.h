// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TileData.h"
#include "FOWTileMap.generated.h"


UCLASS()
class LEAGUEOFLEGENDS_API AFOWTileMap : public AActor
{
	GENERATED_BODY()

public:
	AFOWTileMap();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Tick(float DeltaTime) override;
	
public:
	UFUNCTION(BlueprintCallable)
	void GenerateFromMap(AActor* MapActor);
	
	FTile* GetTile(int32 X, int32 Y);
	const FTile* GetTile(int32 X, int32 Y) const;
	void SetTile(int32 X, int32 Y, const FTile& NewTile);
	
	bool IsValidRange(int32 X, int32 Y) const;
	bool IsInMap(int32 X, int32 Y) const;
	bool IsVisibleTile(int32 X, int32 Y) const;
	void SetTileVisibility(int32 X, int32 Y, bool bVisible);

private:
	void CreateDebugTexture();
	void UpdateDebugTexture();
	
public:
	static constexpr int32 MapSize = 128;
	
protected:
	UPROPERTY()
	float TileSize = -1.f; // 음수면 유효 하지 않음.

	UPROPERTY()
	FVector WorldMin;
	
	UPROPERTY()
	TArray<FTile> Tiles;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<AStaticMeshActor> DebugPlane; // 에디터에서 Plane 연결

	UPROPERTY(EditAnywhere)
	TObjectPtr<UMaterialInterface> DebugMaterial; // 에디터에서 M_TileMapDebug 연결

	UPROPERTY()
	UMaterialInstanceDynamic* DebugMID;
	
	UPROPERTY()
	TObjectPtr<UTexture2D> DebugTexture;
	
	uint8* PixelBuffer = nullptr;
	uint32 PixelBufferSize = 0;
};
