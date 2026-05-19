#include "GameFramework/RiftPlayerCameraManager.h"

ARiftPlayerCameraManager::ARiftPlayerCameraManager()
{
	DefaultFOV = CamFOV;
}

void ARiftPlayerCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	Super::UpdateViewTarget(OutVT, DeltaTime);

	if (!PCOwner || !PCOwner->IsLocalController()) { return; }

	// CameraActor의 SpringArm(-55, 90, 0) + ArmLength 2000과 동일한 계산
	const FRotator CamRot(CamPitch, CamYaw, 0.f);
	const FVector CamOffset = -CamRot.Vector() * ArmLength;

	OutVT.POV.Location = CurrentCameraLoc + CamOffset;
	OutVT.POV.Rotation = CamRot;
	OutVT.POV.FOV = CamFOV;

	// 사망 시 화면 회색조 후처리
	if (bDeathDesaturation)
	{
		OutVT.POV.PostProcessBlendWeight = 1.f;
		OutVT.POV.PostProcessSettings.bOverride_ColorSaturation = true;
		OutVT.POV.PostProcessSettings.ColorSaturation = FVector4(0.f, 0.f, 0.f, 1.f);
	}
}
