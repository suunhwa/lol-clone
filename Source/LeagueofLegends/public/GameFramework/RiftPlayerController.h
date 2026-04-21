#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/RiftTypes.h"
#include "RiftPlayerController.generated.h"

UCLASS()
class LEAGUEOFLEGENDS_API ARiftPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ARiftPlayerController();

	// 로비: 소환사 주문 선택 하게
	UFUNCTION(Server, Reliable)
	void Server_SelectSummonerSpells(ESummonerSpell Spell1, ESummonerSpell Spell2);

	// 로비: 라인 선택 하게 ?
	UFUNCTION(Server, Reliable)
	void Server_SelectLane(ELane Lane);
};
