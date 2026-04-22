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

public:
	virtual void Tick(float DeltaTime) override;
	
public:
	UFUNCTION(BlueprintCallable)
	void GenerateFromMap(AActor* MapActor);
	
	FTile* GetTile(int32 X, int32 Y);
	const FTile* GetTile(int32 X, int32 Y) const;
	
	bool IsValidTile(int32 X, int32 Y) const;
	bool IsInMap(int32 X, int32 Y) const;
	bool IsVisibleTile(int32 X, int32 Y) const;
	void SetTileVisibility(int32 X, int32 Y, bool bVisible);

public:
	static constexpr int32 MapSize = 128;
	
	UPROPERTY()
	float TileSize = -1.f; // 음수면 유효 하지 않음.
	
	UPROPERTY()
	TArray<FTile> Tiles;
};
