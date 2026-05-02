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
	 */
	FORCEINLINE ISightProvider* TryGetProvider(UObject* Object)
	{
		return Cast<ISightProvider>(Object);
	}

	FORCEINLINE const ISightProvider* TryGetProvider(const UObject* Object)
	{
		return Cast<ISightProvider>(Object);
	}

	/** UObject가 ISightProvider를 구현하는지 여부를 반환합니다. */
	FORCEINLINE bool ImplementsSightProvider(const UObject* Object)
	{
		return TryGetProvider(Object) != nullptr;
	}

	// ─────────────────────────────────────────────
	//  안전 래퍼 (null 이면 기본값 반환)
	// ─────────────────────────────────────────────

	/**
	 * 시야 원점을 반환합니다.
	 * ISightProvider를 구현하지 않은 Object가 전달되면 ensure 발생 후 FIntPoint::ZeroValue 반환.
	 */
	inline FVector GetSightOrigin(const UObject* Object)
	{
		const ISightProvider* Provider = TryGetProvider(Object);
		ensureMsgf(Provider, TEXT("GetSightOrigin: Object does not implement ISightProvider"));
		return Provider ? Provider->GetSightOrigin() : FVector::ZeroVector;
	}

	/**
	 * 시야 범위를 반환합니다.
	 * ISightProvider를 구현하지 않은 Object가 전달되면 ensure 발생 후 0.f 반환.
	 */
	inline float GetSightRange(const UObject* Object)
	{
		const ISightProvider* Provider = TryGetProvider(Object);
		ensureMsgf(Provider, TEXT("GetSightRange: Object does not implement ISightProvider"));
		return Provider ? Provider->GetSightRange() : 0.f;
	}

	/**
	 * 정적(Static) 시야 제공자인지 여부를 반환합니다.
	 * Object가 nullptr이거나 인터페이스 미구현이면 false를 반환합니다.
	 */
	inline bool IsStatic(const UObject* Object)
	{
		if (const ISightProvider* Provider = TryGetProvider(Object))
		{
			return Provider->IsStatic();
		}
		return false;
	}

	/**
	 * 팀을 반환합니다.
	 * ISightProvider를 구현하지 않은 Object가 전달되면 ensure 발생 후 ERiftTeam::None 반환.
	 */
	inline ERiftTeam GetTeam(const UObject* Object)
	{
		const ISightProvider* Provider = TryGetProvider(Object);
		ensureMsgf(Provider, TEXT("GetTeam: Object does not implement ISightProvider"));
		return Provider ? Provider->GetTeam() : ERiftTeam::None;
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
		const ISightProvider* ProviderA = TryGetProvider(A);
		const ISightProvider* ProviderB = TryGetProvider(B);
		if (!ProviderA || !ProviderB)
		{
			return false;
		}
		return ProviderA->GetTeam() == ProviderB->GetTeam();
	}

	/**
	 * 두 UObject가 적팀인지 확인합니다.
	 * 둘 중 하나라도 ISightProvider가 아니면 false를 반환합니다.
	 */
	inline bool IsEnemy(const UObject* A, const UObject* B)
	{
		const ISightProvider* ProviderA = TryGetProvider(A);
		const ISightProvider* ProviderB = TryGetProvider(B);
		if (!ProviderA || !ProviderB)
		{
			return false;
		}
		return ProviderA->GetTeam() != ProviderB->GetTeam();
	}

	// ─────────────────────────────────────────────
	//  거리 / 시야 체크 헬퍼
	// ─────────────────────────────────────────────

	/**
	 * Observer의 시야 원점 기준으로 TargetTile이 시야 범위 내에 있는지 확인합니다.
	 * 거리 계산은 타일 좌표(FIntPoint) 기준 XY 평면으로 수행합니다.
	 * @param Observer  시야를 제공하는 UObject (ISightProvider)
	 * @param TargetTile  확인할 타일 좌표
	 * @return 시야 범위 내이면 true, Observer가 유효하지 않으면 false
	 */
	inline bool IsInSightRange(const UObject* Observer, const FVector& TargetTile)
	{
		const ISightProvider* Provider = TryGetProvider(Observer);
		if (!Provider)
		{
			return false;
		}
		const FVector Origin = Provider->GetSightOrigin();
		const float DX = static_cast<float>(Origin.X - TargetTile.X);
		const float DY = static_cast<float>(Origin.Y - TargetTile.Y);
		const float RangeSq = FMath::Square(Provider->GetSightRange());
		return (DX * DX + DY * DY) <= RangeSq;
	}

	/**
	 * Observer의 시야 원점 기준으로 Target UObject가 시야 범위 내에 있는지 확인합니다.
	 * Target도 ISightProvider를 구현해야 합니다.
	 * 거리 계산은 타일 좌표(FIntPoint) 기준 XY 평면으로 수행합니다.
	 */
	inline bool IsInSightRange(const UObject* Observer, const UObject* Target)
	{
		if (!ImplementsSightProvider(Target))
		{
			return false;
		}
		return IsInSightRange(Observer, GetSightOrigin(Target));
	}
} // namespace SightProviderHelper
