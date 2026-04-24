// Fill out your copyright notice in the Description page of Project Settings.


#include "FOW/FOWManager.h"

#include "FOW/FOWTileMap.h"


// Sets default values
AFOWManager::AFOWManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AFOWManager::BeginPlay()
{
	Super::BeginPlay();
	
}

void AFOWManager::ComputeFOV(const FIntPoint& Origin, AFOWTileMap* TileMap)
{
}

void AFOWManager::Scan(FRow Row, const FQuadrant& Quadrant, AFOWTileMap* TileMap)
{
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
		
		if (!bHasPrev)
		{
			if (IsWall(PrevTilePoint, Quadrant, TileMap) && IsFloor(CurTilePoint, Quadrant, TileMap))
			{
				Row.StartSlop = FFraction{ Col * 2 - 1, Row.Depth * 2 };
			}
			else if (IsFloor(PrevTilePoint, Quadrant, TileMap) && IsWall(CurTilePoint, Quadrant, TileMap))
			{
				FRow NextRow = Row.Next();
				NextRow.EndSlop = FFraction{ Col * 2 - 1, Row.Depth * 2 };
				Scan(NextRow, Quadrant, TileMap);
			}
		}
		
		bHasPrev = true;
		PrevTilePoint = CurTilePoint;
	}
	
	if (IsFloor(PrevTilePoint, Quadrant, TileMap))
	{
		Scan(Row.Next(), Quadrant, TileMap);
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

