// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class AFOWTileMap;

class LEAGUEOFLEGENDS_API FFOWUpscaler
{
public:
	static constexpr int32 ScaleFactor = 4;
	
	FFOWUpscaler(int32 InSourceSize);
	~FFOWUpscaler();
	
	void Upscale(const AFOWTileMap* TileMap);
	
	uint8* GetBuffer() const { return UpscaledBuffer; }
	int32 GetUpscaledSize() const { return UpscaledSize; }
	
private:
	int32 SourceSize;
	int32 UpscaledSize;
	uint8* UpscaledBuffer = nullptr;
	
	static const uint8 MarchingPatterns[16][4][4];
};
