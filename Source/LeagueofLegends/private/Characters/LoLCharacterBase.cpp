// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/LoLCharacterBase.h"

#include "LeagueofLegends.h"
#include "Animation/WidgetAnimation.h"
#include "Net/UnrealNetwork.h"
#include "Components/StatComponent.h"
#include "Components/CombatComponent.h"
#include "Components/TagComponent.h"
#include "Components/StateComponent.h"
#include "Components/StatusEffectComponent.h"
#include "Components/CooldownComponent.h"
#include "Components/SkillComponent.h"
#include "Components/TargetingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "FOW/FOWManager.h"
#include "GameFramework/RiftGameState.h"
#include "GameFramework/RiftPlayerState.h"
#include "Components/WidgetComponent.h"
#include "UI/View/HPBarWidget.h"
#include "UI/Widget/PlayerHUDWidget.h"
#include "GameFramework/PlayerState.h"
#include "Misc/OutputDeviceNull.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"    
#include "Components/Image.h"


ALoLCharacterBase::ALoLCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Champion"));

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);

	StatComp = CreateDefaultSubobject<UStatComponent>(TEXT("StatComp"));
	CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComp"));
	TagComp = CreateDefaultSubobject<UTagComponent>(TEXT("TagComp"));
	StateComp = CreateDefaultSubobject<UStateComponent>(TEXT("StateComp"));
	StatusEffectComp = CreateDefaultSubobject<UStatusEffectComponent>(TEXT("StatusEffectComp"));
	CooldownComp = CreateDefaultSubobject<UCooldownComponent>(TEXT("CooldownComp"));
	SkillComp = CreateDefaultSubobject<USkillComponent>(TEXT("SkillComp"));
	TargetingComp = CreateDefaultSubobject<UTargetingComponent>(TEXT("TargetingComp"));

	HPBarWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBarWidgetComp"));
	HPBarWidgetComp->SetupAttachment(GetRootComponent());
	HPBarWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	HPBarWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	HPBarWidgetComp->SetDrawSize(FVector2D(100.f, 12.f));
	
	// ─── 플로팅 텍스트 위젯 컴포넌트 초기화 ───
	FloatingTextWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("FloatingTextWidgetComp"));
	FloatingTextWidgetComp->SetupAttachment(GetRootComponent());
	FloatingTextWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 180.f)); 
	FloatingTextWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	FloatingTextWidgetComp->SetDrawSize(FVector2D(300.f, 100.f)); 
	FloatingTextWidgetComp->SetVisibility(false);
}

void ALoLCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// [FacingRotation 제거] SetReplicateMovement(true)가 회전 복제를 이미 처리하므로 중복.
	// SetActorRotation() sweep이 CMC와 충돌하여 상대 캐릭터 공중 부유 유발.
	// DOREPLIFETIME(ALoLCharacterBase, FacingRotation);
	DOREPLIFETIME(ALoLCharacterBase, FOWVisibilityFlags);
}

// [FacingRotation 제거] SetReplicateMovement(true)가 회전 복제를 이미 처리하므로 중복.
// SetActorRotation() sweep이 CMC와 충돌하여 상대 캐릭터 공중 부유 유발.
// void ALoLCharacterBase::FaceRotation(FRotator NewControlRotation, float DeltaTime)
// {
// 	Super::FaceRotation(NewControlRotation, DeltaTime);
// 
// 	if (HasAuthority())
// 	{
// 		FacingRotation = GetActorRotation();
// 	}
// }

void ALoLCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (HasAuthority())
	{
		if (ARiftPlayerState* PS = GetPlayerState<ARiftPlayerState>())
		{
			TagComp->SetTeam(PS->GetTeam());
		}
	}
}

void ALoLCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		// TagComp->SetTeam(InitialTeam);
		CombatComp->OnDeath.AddUObject(this, &ALoLCharacterBase::OnDeath);
	}

	/*auto* GS = GetWorld()->GetGameState<ARiftGameState>();                                                                                  
	GS->GetFOWManager()->RegisterSightProvider(this);    */
	
	if (auto* GS = GetWorld()->GetGameState<ARiftGameState>())
	{
		if (AFOWManager* FOWManager = GS->GetFOWManager())
		{
			FOWManager->RegisterSightProvider(this);
		}
		else
		{
			PRINTLOG_TK(TEXT("RegisterSightProvider failed: FOWManager is null for %s"), *GetName());
		}
	}
	else
	{
		PRINTLOG_TK(TEXT("RegisterSightProvider failed: GameState is null for %s"), *GetName());
	}

	if (UHPBarWidget* HPBar = Cast<UHPBarWidget>(HPBarWidgetComp->GetWidget()))
	{
		HPBar->InitWidget(StatComp);
	}

	InitPlayerHUDWidget();
	
	// 초기 가시성 상태 적용 (FOWVisibilityFlags = 0이므로 적군은 숨김 처리)
	OnRep_FOWVisibility();
}

void ALoLCharacterBase::ReceiveDamage_Implementation(float Amount, EDamageType DamageType, AActor* DamageInstigator)
{
	if (!HasAuthority()) { return; }

	FDamageContext Ctx;
	Ctx.RawDamage = Amount;
	Ctx.DamageType = DamageType;
	Ctx.DamageInstigator = DamageInstigator;

	if (DamageInstigator)
	{
		UCombatComponent* InstigatorCombat = DamageInstigator->FindComponentByClass<UCombatComponent>();
		if (InstigatorCombat)
		{
			InstigatorCombat->DealDamage(this, Ctx);
			return;
		}
	}

	// InstigatorCombat 없는 경우 (포탑 투사체 등) 직접 처리
	StatComp->ApplyHealthChange(-Amount);

	if (StatComp->IsDead())
	{
		StateComp->TryChangeState(ECharacterState::Dead);
		TagComp->AddTag(UnitTags::Dead);
		TagComp->AddTag(UnitTags::Untargetable);
		CombatComp->OnDeath.Broadcast(Ctx.DamageInstigator);
	}
}

bool ALoLCharacterBase::IsDead_Implementation() const
{
	return StatComp->IsDead();
}

bool ALoLCharacterBase::IsTargetable_Implementation() const
{
	return !IsDead_Implementation() && !TagComp->HasTag(UnitTags::Untargetable);
}

FVector ALoLCharacterBase::GetTargetLocation_Implementation() const
{
	return GetActorLocation();
}

ETeam ALoLCharacterBase::GetTeam_Implementation() const
{
	return TagComp->GetTeam();
}

EUnitType ALoLCharacterBase::GetUnitType_Implementation() const
{
	return TagComp->GetUnitType();
}

AActor* ALoLCharacterBase::GetCurrentCombatTarget_Implementation() const
{
	return TargetingComp ? TargetingComp->GetCurrentTarget() : nullptr;
}

FVector ALoLCharacterBase::GetSightOrigin_Implementation() const
{
	return GetActorLocation();
}

float ALoLCharacterBase::GetSightRange_Implementation() const
{
	return SightRange;
}

bool ALoLCharacterBase::IsStatic_Implementation() const
{
	return bStaticSight;
}

ERiftSightTag ALoLCharacterBase::GetSightTag_Implementation() const
{
	return TagComp->GetSightTag();
}

bool ALoLCharacterBase::IsHideable_Implementation() const
{
	return false; 
}

void ALoLCharacterBase::SetFOWVisibilityFlag_Implementation(ERiftSightTag Team, bool bVisible)
{
	if (!HasAuthority()) // 서버에서만 호출됨
	{
		return;
	}
	
	uint8 Mask = 0;
	if (Team == ERiftSightTag::Red)
	{
		Mask = 0x01;
	}
	else if (Team == ERiftSightTag::Blue)
	{
		Mask = 0x02;
	}
	
	uint8 OldFlags = FOWVisibilityFlags;
	
	if (bVisible)
	{
		FOWVisibilityFlags |= Mask;
	}
	else
	{
		FOWVisibilityFlags &= ~Mask;
	}
	
	// 값이 바뀌었고, Host인 경우 수동 호출
	if (OldFlags != FOWVisibilityFlags)
	{
		// UE_LOG(LogTemp, Warning, TEXT("[FOW] %s | Team=%d bVisible=%d OldFlags=%d NewFlags=%d"),
		//    *GetName(), (int32)Team, bVisible, OldFlags, FOWVisibilityFlags);
		OnRep_FOWVisibility();
	}
}

void ALoLCharacterBase::OnRep_FOWVisibility()
{
	// 로컬 플레이어의 팀 가져오기
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->GetPawn())
	{
		return;
	}

	ALoLCharacterBase* LocalCharacter = Cast<ALoLCharacterBase>(PC->GetPawn());
	if (!LocalCharacter) 
	{
		return;
	}

	ERiftSightTag LocalClientTeam = LocalCharacter->TagComp->GetSightTag();

	if (TagComp->GetSightTag() == LocalClientTeam)
	{
		// 같은 팀 오브젝트는 항상 보이도록 설정
		SetActorHiddenInGame(false);
		return;
	}
	
	// 적군만 가시성 판정
	bool bVisibleToMe = false;
	if (LocalClientTeam == ERiftSightTag::Red)
	{
		bVisibleToMe = (FOWVisibilityFlags & 0x01) != 0;
	}
	else if (LocalClientTeam == ERiftSightTag::Blue)
	{
		bVisibleToMe = (FOWVisibilityFlags & 0x02) != 0;
	}

	// UE_LOG(LogTemp, Warning, TEXT("[FOW OnRep] %s | MyTeam=%d EnemyTeam=%d Flags=%d Hidden=%d"),
	//    *GetName(), (int32)LocalClientTeam, (int32)TagComp->GetSightTag(), FOWVisibilityFlags, !bVisibleToMe);
	
	SetActorHiddenInGame(!bVisibleToMe);
}

void ALoLCharacterBase::OnDeath(AActor* DamageInstigator)
{
	Multicast_OnDeath();
}

void ALoLCharacterBase::Multicast_StartCooldown_Implementation(FName Tag, float Duration)
{
	// 서버는 StartCooldown에서 이미 처리했으므로 클라이언트만 적용
	if (!HasAuthority() && CooldownComp)
		CooldownComp->StartCooldown(Tag, Duration);
}

void ALoLCharacterBase::Multicast_PlayMontage_Implementation(UAnimMontage* Montage)
{
	if (!Montage) { return; }
	PlayAnimMontage(Montage);
}

void ALoLCharacterBase::Multicast_PlayMontageSection_Implementation(UAnimMontage* Montage, FName SectionName)
{
	if (!Montage) { return; }
	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	if (!Anim) { return; }
	Anim->Montage_Play(Montage);
	Anim->Montage_JumpToSection(SectionName, Montage);
}

void ALoLCharacterBase::Multicast_OnDeath_Implementation()
{
	SetActorEnableCollision(false);
	if (HPBarWidgetComp)
	{
		HPBarWidgetComp->SetVisibility(false);
	}
}

void ALoLCharacterBase::InitPlayerHUDWidget()
{
	if (!HPBarWidgetComp) return;

	UPlayerHUDWidget* HUDWidget = Cast<UPlayerHUDWidget>(HPBarWidgetComp->GetWidget());
	if (!HUDWidget) return;

	if (!StatComp) return;

	// 초기 값 세팅
	HUDWidget->SetLevel(StatComp->GetLevel());
	HUDWidget->SetHP(StatComp->GetCurrentHP(), StatComp->GetMaxHP());
	HUDWidget->SetMP(StatComp->GetCurrentMana(), StatComp->GetMaxMana());
	HUDWidget->SetMaxHP(StatComp->GetMaxHP());

	// 안전한 약참조로 람다 바인딩 (중복 구독 방지: BeginPlay에서 1회만 호출)
	TWeakObjectPtr<UPlayerHUDWidget> WeakHUD(HUDWidget);

	StatComp->OnHPChanged.AddLambda([WeakHUD](float Current, float Max)
	{
		if (WeakHUD.IsValid())
		{
			WeakHUD->SetHP(Current, Max);
			WeakHUD->SetMaxHP(Max);
		}
	});

	StatComp->OnManaChanged.AddLambda([WeakHUD](float Current, float Max)
	{
		if (WeakHUD.IsValid())
			WeakHUD->SetMP(Current, Max);
	});

	StatComp->OnLevelChanged.AddLambda([WeakHUD](int32 NewLevel)
	{
		if (WeakHUD.IsValid())
			WeakHUD->SetLevel(NewLevel);
	});

	// PlayerState가 이미 있으면 즉시 갱신, 없으면 OnRep_PlayerState에서 갱신
	RefreshHUDDisplay();

	// 닉네임 변경 구독 (ServerChangeName RPC 결과가 늦게 도착하는 경우 대응)
	if (ARiftPlayerState* MyPS = GetPlayerState<ARiftPlayerState>())
	{
		MyPS->OnNameChanged.AddWeakLambda(this, [WeakHUD](const FString& Name)
		{
			if (WeakHUD.IsValid())
				WeakHUD->SetNickName(Name);
		});
	}
}

void ALoLCharacterBase::RefreshHUDDisplay()
{
	if (!HPBarWidgetComp) return;
	UPlayerHUDWidget* HUDWidget = Cast<UPlayerHUDWidget>(HPBarWidgetComp->GetWidget());
	if (!HUDWidget) return;

	// 닉네임: PlayerState에서 현재 값 읽기 (구독은 InitPlayerHUDWidget에서 1회만)
	if (ARiftPlayerState* MyPS = GetPlayerState<ARiftPlayerState>())
	{
		const FString Name = MyPS->GetPlayerName();
		if (!Name.IsEmpty())
		{
			HUDWidget->SetNickName(Name);
		}
	}

	// HP바 색상: 컨트롤러 기반으로 Self 판별, PlayerState 팀으로 Ally/Enemy 판별
	EHPBarType HPBarType = EHPBarType::Enemy;

	AController* MyCtrl = GetController();
	if (MyCtrl && MyCtrl->IsLocalController())
	{
		HPBarType = EHPBarType::Self;
	}
	else
	{
		ARiftPlayerState* MyPS = GetPlayerState<ARiftPlayerState>();
		APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
		APawn* LocalPawn = LocalPC ? LocalPC->GetPawn() : nullptr;
		ARiftPlayerState* LocalPS = LocalPawn ? LocalPawn->GetPlayerState<ARiftPlayerState>() : nullptr;

		if (MyPS && LocalPS &&
			MyPS->GetTeam() != ETeam::None &&
			LocalPS->GetTeam() != ETeam::None &&
			MyPS->GetTeam() == LocalPS->GetTeam())
		{
			HPBarType = EHPBarType::Ally;
		}
	}

	HUDWidget->SetHPBarType(HPBarType);
}

void ALoLCharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (ARiftPlayerState* PS = GetPlayerState<ARiftPlayerState>())
	{
		// 클라이언트: TagComp 팀 동기화 (서버는 PossessedBy에서 처리)
		if (TagComp && PS->GetTeam() != ETeam::None)
		{
			TagComp->SetTeam(PS->GetTeam());
		}

		// BeginPlay 시점에 PS가 null이어서 구독 못 한 경우 여기서 보완
		if (HPBarWidgetComp)
		{
			if (UPlayerHUDWidget* HUDWidget = Cast<UPlayerHUDWidget>(HPBarWidgetComp->GetWidget()))
			{
				TWeakObjectPtr<UPlayerHUDWidget> WeakHUD(HUDWidget);
				PS->OnNameChanged.AddWeakLambda(this, [WeakHUD](const FString& Name)
				{
					if (WeakHUD.IsValid())
						WeakHUD->SetNickName(Name);
				});
			}
		}
	}

	RefreshHUDDisplay();
}

void ALoLCharacterBase::Client_CreateFloatingText_Implementation(int32 Amount, bool bIsGold, FVector SpawnLocation)
{
    if (!FloatingTextWidgetComp) return;
	
	// 부모 캐릭터의 이동, 회전, 스케일 상속을 완전히 끊어버려, 컴포넌트가 월드 공간에 완전히 독립적으로 고정
	FloatingTextWidgetComp->SetUsingAbsoluteLocation(true);
	FloatingTextWidgetComp->SetUsingAbsoluteRotation(true);
    // 📍 [핵심 변경] 골드든 경험치든 인자로 넘어온 월드 좌표에 위젯 컴포넌트를 순간이동 시킵니다!
    // 이렇게 하면 챔피언에 달린 컴포넌트라 하더라도 완벽하게 미니언 시체 머리 위 허공에 고정됩니다.
    FloatingTextWidgetComp->SetWorldLocation(SpawnLocation);

    // 1. 위젯 컴포넌트 강제 활성화
    FloatingTextWidgetComp->SetVisibility(true);

    // 2. 컴포넌트에 꽂힌 실제 UserWidget 객체 반환
    UUserWidget* TextWidget = FloatingTextWidgetComp->GetUserWidgetObject();
    if (!TextWidget) return;

    UTextBlock* AmountTextBlock = Cast<UTextBlock>(TextWidget->GetWidgetFromName(TEXT("Txt_Amount")));
    UImage* IconImage = Cast<UImage>(TextWidget->GetWidgetFromName(TEXT("Img_Icon"))); 
    
    if (AmountTextBlock)
    {
        if (bIsGold)
        {
            FString GoldStr = FString::Printf(TEXT("+%d"), Amount);
            AmountTextBlock->SetText(FText::FromString(GoldStr));
            FLinearColor GoldColor(1.0f, 0.85f, 0.0f, 1.0f);
            AmountTextBlock->SetColorAndOpacity(FSlateColor(GoldColor));

            if (IconImage)
            {
                UTexture2D* GoldTex = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, TEXT("/Game/Asset/UI/TopBar/nav-icon-store_waifu2x_art_noise1_scale.nav-icon-store_waifu2x_art_noise1_scale")));
                if (GoldTex)
                {
                    IconImage->SetBrushFromTexture(GoldTex);
                }
            }
        }
        else
        {
            FString XPStr = FString::Printf(TEXT("+%d XP"), Amount);
            AmountTextBlock->SetText(FText::FromString(XPStr));
            FLinearColor XPColor(0.6f, 0.3f, 1.0f, 1.0f);
            AmountTextBlock->SetColorAndOpacity(FSlateColor(XPColor));

            if (IconImage)
            {
                UTexture2D* XPTex = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, TEXT("/Game/Asset/UI/Common/npe-rewards-xp-boost.npe-rewards-xp-boost")));
                if (XPTex)
                {
                    IconImage->SetBrushFromTexture(XPTex);
                }
            }
        }
    }

    // 🎬 3. 블루프린트 노드 없이 "PopUp" 애니메이션 C++ 강제 구동
    FProperty* AnimProp = TextWidget->GetClass()->FindPropertyByName(TEXT("PopUp"));
    if (AnimProp)
    {
        if (FObjectProperty* ObjProp = CastField<FObjectProperty>(AnimProp))
        {
            if (UWidgetAnimation* PopUpAnim = Cast<UWidgetAnimation>(ObjProp->GetObjectPropertyValue_InContainer(TextWidget)))
            {
                TextWidget->PlayAnimation(PopUpAnim);
            }
        }
    }

    PRINTLOG_HJ(TEXT("[Floating UI] 좌표 순간이동 및 UI 처리 완수 ➔ X:%f, Y:%f, Z:%f"), SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);
}