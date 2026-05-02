// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FOWManager.generated.h"

struct FTile;
class AFOWTileMap;

UENUM()
enum class EQuadrantDirection : uint8
{
	North,
	East, 
	South,
	West, 
};

USTRUCT()
struct FQuadrant
{
	GENERATED_BODY()
	
	EQuadrantDirection Direction;
	FIntPoint Origin;

	FIntPoint Transform(int32 Row, int32 Col) const
	{
		switch (Direction)
		{
		case EQuadrantDirection::North:
			return FIntPoint(Origin.X + Col, Origin.Y - Row);
		case EQuadrantDirection::East:
			return FIntPoint(Origin.X + Row, Origin.Y + Col);
		case EQuadrantDirection::South:
			return FIntPoint(Origin.X + Col, Origin.Y + Row);
		case EQuadrantDirection::West:
			return FIntPoint(Origin.X - Row, Origin.Y + Col);
		default:
			return FIntPoint::ZeroValue;
		}
	}
};

USTRUCT()
struct FFraction
{
	GENERATED_BODY()
	
	int32 Numerator = 0; // 분자
	int32 Denominator = 1; // 분모
	
	int32 RoundTiesUp() const
	{
		// floor((2*Numerator + Denominator) / (2*Denominator))
		int32 Num = 2 * Numerator + Denominator;
		int32 Den = 2 * Denominator;
		return FMath::FloorToInt(static_cast<float>(Num) / Den);
	}

	int32 RoundTiesDown() const
	{
		// ceil((2*Numerator - Denominator) / (2*Denominator))
		int32 Num = 2 * Numerator - Denominator;
		int32 Den = 2 * Denominator;
		return FMath::CeilToInt(static_cast<float>(Num) / Den);
	}
};

USTRUCT()
struct FRow
{
	GENERATED_BODY()

	FRow() : Depth(0), StartSlop(FFraction{-1, 1}), EndSlop(FFraction{1, 1})
	{
	}
	
	FRow(int32 InDepth, FFraction InStartSlop, FFraction InEndSlop)
		: Depth(InDepth), StartSlop(InStartSlop), EndSlop(InEndSlop)
	{
	}
	
	int32 Depth = 1;
	FFraction StartSlop{-1, 1};
	FFraction EndSlop{1, 1};
	
	FRow Next()
	{
		return FRow(Depth + 1, StartSlop, EndSlop);
	}

	int32 GetMinCol() const
	{
		// depth * start_slope
		FFraction F{ Depth * StartSlop.Numerator, StartSlop.Denominator };
		return F.RoundTiesUp();
	}

	int32 GetMaxCol() const
	{
		// depth * end_slope
		FFraction F{ Depth * EndSlop.Numerator, EndSlop.Denominator };
		return F.RoundTiesDown();
	}
};

UCLASS()
class LEAGUEOFLEGENDS_API AFOWManager : public AActor	
{
	GENERATED_BODY()

public:
	AFOWManager();

protected:
	virtual void BeginPlay() override;
	
public:
	virtual void Tick(float DeltaTime) override;

public:
	// 플레이어 위치를 원점으로 하는 시야 계산
	UFUNCTION()
	void ComputeFOV(const FIntPoint& Origin, AFOWTileMap* TileMap);
	
private:
	UFUNCTION()
	void Scan(FRow Row, const FQuadrant& Quadrant, AFOWTileMap* TileMap);
	
	UFUNCTION()
	bool IsWall(FIntPoint& Tile, const FQuadrant& Quadrant, AFOWTileMap* TileMap) const;

	UFUNCTION()
	bool IsFloor(FIntPoint& Tile, const FQuadrant& Quadrant, AFOWTileMap* TileMap) const;

	UFUNCTION()
	void Reveal(FIntPoint& Tile, const FQuadrant& Quadrant, AFOWTileMap* TileMap);
	
	UFUNCTION()
	bool IsSymmetric(FRow& Row, int32 Depth, int32 Col) const;
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TileMap")
	TObjectPtr<AFOWTileMap> RedTileMap;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TileMap")
	TObjectPtr<AFOWTileMap> BlueTileMap;
	
private:
	// TODO: Acter 캐싱 대신 interface로 변경 (ISightProvider)
	UPROPERTY()
	TArray<AActor*> RedSightActor;
	
	UPROPERTY()
	TArray<AActor*> BlueSightActor;
	
#pragma region Test
	// UPROPERTY(EditAnywhere)
	// AActor* TestActor;  // 에디터에서 챔피언 하나 연결
	//
	// UPROPERTY(EditAnywhere)
	// AFOWTileMap* TestTileMap;  // 에디터에서 TileMap 연결
	//
	// UPROPERTY(EditAnywhere)
	// int32 SightRadius = 10;  // 테스트용 시야 반경
#pragma endregion
};
