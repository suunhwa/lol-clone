// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WidgetViewBase.h"
#include "MinimapWidget.generated.h"

class UOverlay;
class AFOWTileMap;
class UImage;
/**
 * 
 */
UCLASS()
class LEAGUEOFLEGENDS_API UMinimapWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	void SetMinimapFOWTexture(UTexture2D* NewTexture);

	void UpdateViewRect(APlayerController* PC, AFOWTileMap* TileMap);
	void SetLocalTileMap(AFOWTileMap* AfowTileMap) { CachedTileMap = AfowTileMap; }

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_MinimapFOW;
	
	UPROPERTY(EditAnywhere, Category = "Minimap")
	TObjectPtr<UMaterialInterface> FOWMaterial;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> FOWMID;
	
	// Camera Area
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> Overlay_ViewRect; // 카메라 영역 표시용
	
	UPROPERTY()
	TObjectPtr<AFOWTileMap> CachedTileMap;
};
