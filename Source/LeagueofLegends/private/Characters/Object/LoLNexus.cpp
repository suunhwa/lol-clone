#include "Characters/Object/LoLNexus.h"

#include "LeagueofLegends.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"

ALoLNexus::ALoLNexus()
{
	ObjectID = 11111; // 넥서스 고유 ID
}

void ALoLNexus::OnDestroyed()
{
	// 1. 기본 파괴 로직 수행 (bIsDestroyed 설정 등)
	Super::OnDestroyed();

	// 2. 물리 및 렌더링 끄기
	SetActorEnableCollision(false);
	if (MeshComp)
	{
		MeshComp->SetHiddenInGame(true);
	}

	// 3. 게임 종료 처리
	HandleGameOver();
    
	PRINTLOG_HJ(TEXT("[넥서스] 파괴됨! %s 팀의 승리입니다!"), (GetTeam() == ETeam::Red) ? TEXT("Blue") : TEXT("Red"));
}

void ALoLNexus::HandleGameOver()
{
	// [알파 연출 1] 화려한 폭발 이펙트 (에디터에 있는 기본 이펙트 아무거나 써도 됨)
	// 넥서스 위치에서 크게 터뜨리기
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), nullptr, GetActorLocation(), GetActorRotation(), FVector(5.0f));

	// [알파 연출 2] 슬로우 모션 (롤 끝날 때 느낌)
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.2f);

	// [알파 연출 3] 3초 뒤에 게임 정지 또는 결과 UI (지금은 로그로 대체하거나 메시지 박스)
	// 실제로는 여기서 Victory/Defeat UI를 띄워야 해.
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, [this]()
	{
		// 게임을 일시정지시키거나 메인 화면으로 이동
		// UGameplayStatics::SetGamePaused(GetWorld(), true);
	}, 3.0f, false);
}