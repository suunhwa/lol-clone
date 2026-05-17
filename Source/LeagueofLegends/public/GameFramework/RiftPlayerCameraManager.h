#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "RiftPlayerCameraManager.generated.h"

UCLASS()
class LEAGUEOFLEGENDS_API ARiftPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	ARiftPlayerCameraManager();

	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;

	// PlayerController Tick에서 매 프레임 갱신하는 지면 기준 카메라 위치
	FVector CurrentCameraLoc = FVector::ZeroVector;

	// 팀에 따른 Yaw 설정 (블루=90°, 레드=-90°)
	float CamYaw = 90.f;
	void SetTeamYaw(bool bRedTeam) { CamYaw = bRedTeam ? -90.f : 90.f; }

private:
	static constexpr float ArmLength = 2000.f;
	static constexpr float CamPitch  = -55.f;
	static constexpr float CamFOV    = 60.f;
};
