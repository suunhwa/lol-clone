// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SightProvider.h"

/**
 * ISightProvider 인터페이스의 헬퍼 함수 모음.
 * 호출부의 캐스팅 / null 체크를 숨겨 코드를 간결하게 유지합니다.
 */
namespace SightProviderHelper
{
	/**
	 * UObject가 ISightProvider를 구현하는지 확인합니다.
	 * @return 인터페이스 포인터 (미구현이면 nullptr)
	 * @note BP 구현체는 Cast<ISightProvider>가 null을 반환할 수 있으므로
	 *       인터페이스 함수 호출은 Execute_ 방식을 사용해야 합니다.
	 */
	FORCEINLINE ISightProvider* TryGetProvider(UObject* Object)
	{
		if (!Object || !Object->Implements<USightProvider>())
		{
			return nullptr;
		}
		return Cast<ISightProvider>(Object);
	}

	FORCEINLINE const ISightProvider* TryGetProvider(const UObject* Object)
	{
		if (!Object || !Object->Implements<USightProvider>())
		{
			return nullptr;
		}
		return Cast<ISightProvider>(Object);
	}

	/** UObject가 ISightProvider를 구현하는지 여부를 반환합니다. */
	FORCEINLINE bool IsSightProvider(const UObject* Object)
	{
		return Object && Object->Implements<USightProvider>();
	}

	// ─────────────────────────────────────────────
	//  안전 래퍼 (null 이면 기본값 반환)
	// ─────────────────────────────────────────────

	/**
	 * 시야 원점을 반환합니다.
	 * ISightProvider를 구현하지 않은 Object가 전달되면 ensure 발생 후 FVector::ZeroVector 반환.
	 * @note BlueprintNativeEvent이므로 Execute_ 방식으로 호출합니다. (BP 구현체 지원)
	 */
	inline FVector GetSightOrigin(const UObject* Object)
	{
		ensureMsgf(IsSightProvider(Object), TEXT("GetSightOrigin: Object does not implement ISightProvider"));
		if (!IsSightProvider(Object)) return FVector::ZeroVector;
		return ISightProvider::Execute_GetSightOrigin(Object);
	}

	/**
	 * 시야 범위를 반환합니다.
	 * ISightProvider를 구현하지 않은 Object가 전달되면 ensure 발생 후 0.f 반환.
	 * @note BlueprintNativeEvent이므로 Execute_ 방식으로 호출합니다. (BP 구현체 지원)
	 */
	inline float GetSightRange(const UObject* Object)
	{
		ensureMsgf(IsSightProvider(Object), TEXT("GetSightRange: Object does not implement ISightProvider"));
		if (!IsSightProvider(Object)) return 0.f;
		return ISightProvider::Execute_GetSightRange(Object);
	}

	/**
	 * 정적(Static) 시야 제공자인지 여부를 반환합니다.
	 * Object가 nullptr이거나 인터페이스 미구현이면 false를 반환합니다.
	 * @note BlueprintNativeEvent이므로 Execute_ 방식으로 호출합니다. (BP 구현체 지원)
	 */
	inline bool IsStatic(const UObject* Object)
	{
		if (!IsSightProvider(Object)) return false;
		return ISightProvider::Execute_IsStatic(Object);
	}

	/**
	 * 팀을 반환합니다.
	 * ISightProvider를 구현하지 않은 Object가 전달되면 ensure 발생 후 ERiftTeam::None 반환.
	 * @note BlueprintNativeEvent이므로 Execute_ 방식으로 호출합니다. (BP 구현체 지원)
	 */
	inline ERiftSightTag GetTeam(const UObject* Object)
	{
		ensureMsgf(IsSightProvider(Object), TEXT("GetTeam: Object does not implement ISightProvider"));
		if (!IsSightProvider(Object)) return ERiftSightTag::None;
		return ISightProvider::Execute_GetSightTag(Object);
	}

	/**
	 * 비가시 영역에서 숨겨져야 하는지 여부를 반환합니다.
	 * Object가 nullptr이거나 인터페이스 미구현이면 false를 반환합니다.
	 * @note true: 챔피언, 미니언 / false: 타워 등 항상 보이는 오브젝트
	 */
	inline bool IsHideable(const UObject* Object)
	{
		if (!IsSightProvider(Object)) return false;
		return ISightProvider::Execute_IsHideable(Object);
	}
	
	/**
	 * 가시성 여부에 따라 Actor를 숨기거나 표시합니다.
	 * ISightProvider를 구현하지 않았거나 IsHideableByFOW가 false면 아무것도 하지 않습니다.
	 * @param Object    대상 UObject (ISightProvider + AActor)
	 * @param bVisible  true면 표시, false면 숨김
	 */
	inline void ApplyFOWVisibility(UObject* Object, bool bVisible)
	{
		if (!IsHideable(Object)) return;

		if (AActor* Actor = Cast<AActor>(Object))
		{
			Actor->SetActorHiddenInGame(!bVisible);
		}
	}

	// ─────────────────────────────────────────────
	//  팀 비교 헬퍼
	// ─────────────────────────────────────────────

	/**
	 * 두 UObject가 같은 팀인지 확인합니다.
	 * 둘 중 하나라도 ISightProvider가 아니면 false를 반환합니다.
	 */
	inline bool IsSameTeam(const UObject* A, const UObject* B)
	{
		if (!IsSightProvider(A) || !IsSightProvider(B)) return false;
		return ISightProvider::Execute_GetSightTag(A) == ISightProvider::Execute_GetSightTag(B);
	}

	/**
	 * 두 UObject가 적팀인지 확인합니다.
	 * 둘 중 하나라도 ISightProvider가 아니면 false를 반환합니다.
	 */
	inline bool IsEnemy(const UObject* A, const UObject* B)
	{
		if (!IsSightProvider(A) || !IsSightProvider(B)) return false;
		return ISightProvider::Execute_GetSightTag(A) != ISightProvider::Execute_GetSightTag(B);
	}

	// ─────────────────────────────────────────────
	//  거리 / 시야 체크 헬퍼
	// ─────────────────────────────────────────────

	/**
	 * Observer의 시야 원점 기준으로 TargetLocation이 시야 범위 내에 있는지 확인합니다.
	 * 거리 계산은 XY 평면 기준으로 수행합니다.
	 * @param Observer      시야를 제공하는 UObject (ISightProvider)
	 * @param TargetLocation 확인할 월드 좌표
	 * @return 시야 범위 내이면 true, Observer가 유효하지 않으면 false
	 */
	inline bool IsInSightRange(const UObject* Observer, const FVector& TargetLocation)
	{
		if (!IsSightProvider(Observer)) return false;
		const FVector Origin = ISightProvider::Execute_GetSightOrigin(Observer);
		const float DX = Origin.X - TargetLocation.X;
		const float DY = Origin.Y - TargetLocation.Y;
		const float Range = ISightProvider::Execute_GetSightRange(Observer);
		return (DX * DX + DY * DY) <= FMath::Square(Range);
	}

	/**
	 * Observer의 시야 원점 기준으로 Target UObject가 시야 범위 내에 있는지 확인합니다.
	 * Target도 ISightProvider를 구현해야 합니다.
	 * 거리 계산은 XY 평면 기준으로 수행합니다.
	 */
	inline bool IsInSightRange(const UObject* Observer, const UObject* Target)
	{
		if (!IsSightProvider(Target)) return false;
		return IsInSightRange(Observer, GetSightOrigin(Target));
	}
} // namespace SightProviderHelper
