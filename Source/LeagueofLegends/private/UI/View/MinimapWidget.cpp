// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/View/MinimapWidget.h"

#include "LeagueofLegends.h"
#include "Components/CanvasPanelSlot.h"
#include "FOW/FOWTileMap.h"

#include "Components/Image.h"
#include "Components/Overlay.h"

void UMinimapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	APlayerController* PC = GetOwningPlayer();
	// TileMap은 캐싱해두거나 매번 가져오기
	if (CachedTileMap)
	{
		UpdateViewRect(PC, CachedTileMap);
	}
}

void UMinimapWidget::SetMinimapFOWTexture(UTexture2D* NewTexture)
{
	if (!Img_MinimapFOW || !NewTexture || !FOWMaterial)
	{
		return;
	}

	if (!FOWMID)
	{
		FOWMID = UMaterialInstanceDynamic::Create(FOWMaterial, this);
	}

	FOWMID->SetTextureParameterValue(TEXT("FogTexture"), NewTexture);
	Img_MinimapFOW->SetBrushFromMaterial(FOWMID);
	Img_MinimapFOW->SetDesiredSizeOverride(FVector2D(512.f, 512.f));
	Img_MinimapFOW->SetVisibility(ESlateVisibility::HitTestInvisible);
	Img_MinimapFOW->SetRenderOpacity(1.f);
	Img_MinimapFOW->SetBrushTintColor(FLinearColor(1.f, 1.f, 1.f, 0.7f)); // 안개 농도 조절
}

void UMinimapWidget::UpdateViewRect(APlayerController* PC, AFOWTileMap* TileMap)
{
    if (!PC || !TileMap || !Img_MinimapFOW || !Overlay_ViewRect) return;

    int32 ViewportX, ViewportY;
    PC->GetViewportSize(ViewportX, ViewportY);

    FVector TopLeftWorld, TopLeftDir;
    FVector BottomRightWorld, BottomRightDir;

    PC->DeprojectScreenPositionToWorld(0, 0, TopLeftWorld, TopLeftDir);
    PC->DeprojectScreenPositionToWorld(ViewportX, ViewportY, BottomRightWorld, BottomRightDir);

	auto RayToGround = [](const FVector& Origin, const FVector& Dir) -> FVector
	{
		if (FMath::Abs(Dir.Z) < KINDA_SMALL_NUMBER) return Origin;
		float T = (8600.f - Origin.Z) / Dir.Z;
		return Origin + Dir * T;
	};

    FVector GroundTL = RayToGround(TopLeftWorld, TopLeftDir);
    FVector GroundBR = RayToGround(BottomRightWorld, BottomRightDir);

    // 월드 → UV 직접 계산 (타일맵 범위 밖이어도 안전)
    float MapWorldSize = TileMap->GetTileSize() * TileMap->MapSize;
    FVector WorldMin = TileMap->GetWorldMin();

    auto WorldToUV = [&](const FVector& WorldPos) -> FVector2D
    {
        float U = (WorldPos.X - WorldMin.X) / MapWorldSize;
        float V = (WorldPos.Y - WorldMin.Y) / MapWorldSize;
        return FVector2D(
            FMath::Clamp(U, 0.f, 1.f),
            FMath::Clamp(V, 0.f, 1.f));
    };

    FVector2D UVTL = WorldToUV(GroundTL);
    FVector2D UVBR = WorldToUV(GroundBR);
	
	// TL/BR 정렬 (카메라 방향에 따라 뒤집힐 수 있음)
	float MinU = FMath::Min(UVTL.X, UVBR.X);
	float MinV = FMath::Min(UVTL.Y, UVBR.Y);
	float MaxU = FMath::Max(UVTL.X, UVBR.X);
	float MaxV = FMath::Max(UVTL.Y, UVBR.Y);

    FVector2D MinimapSize = Img_MinimapFOW->GetCachedGeometry().GetLocalSize();
    if (MinimapSize.IsNearlyZero())
    {
        MinimapSize = FVector2D(512.f, 512.f);
    }

	float Left   = MinU * MinimapSize.X;
	float Top    = MinV * MinimapSize.Y;
	float Width  = (MaxU - MinU) * MinimapSize.X;
	float Height = (MaxV - MinV) * MinimapSize.Y;

    if (UCanvasPanelSlot* CanvasPanelSlot = Cast<UCanvasPanelSlot>(Overlay_ViewRect->Slot))
    {
        CanvasPanelSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
        CanvasPanelSlot->SetPosition(FVector2D(Left, Top));
        CanvasPanelSlot->SetSize(FVector2D(Width, Height));
        CanvasPanelSlot->SetAlignment(FVector2D(0.f, 0.f));
    	
    	// 값 디버깅 로그
    	PRINTLOG_TK(TEXT("Minimap ViewRect Updated: Left=%.1f Top=%.1f Width=%.1f Height=%.1f"), Left, Top, Width, Height);
    }
}
