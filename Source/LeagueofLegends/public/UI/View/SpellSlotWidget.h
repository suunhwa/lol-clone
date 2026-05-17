#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Type/RiftTypes.h"
#include "SpellSlotWidget.generated.h"

class UImage;
class UTextBlock;
class UOverlay;
class UMaterialInstanceDynamic;

UCLASS()
class LEAGUEOFLEGENDS_API USpellSlotWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime) override;

public:
	void InitSlot(int32 InSlotIndex, ESummonerSpell InSpell);
	void TriggerCooldown(float Duration); // 스펠 시전 시 외부에서 호출

private:
	void RefreshIcon();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_Icon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> Overlay_CD;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_Cooldown;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_CoolTime;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Index;

	// 스펠별 아이콘 텍스처 — Blueprint에서 할당
	UPROPERTY(EditDefaultsOnly, Category = "Spell|Icons")
	TMap<ESummonerSpell, TObjectPtr<UTexture2D>> SpellIcons;

	int32 SlotIndex = 0;
	ESummonerSpell Spell = ESummonerSpell::None;

	// 로컬 타이머 (CooldownComp 의존 제거)
	float LocalRemaining = 0.f;
	float LocalTotalCD   = 1.f;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> CooldownMat;
};
