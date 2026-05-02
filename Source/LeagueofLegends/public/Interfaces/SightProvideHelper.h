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

	// ─────────────────────────────────────────────
	//  안전 래퍼 (null 이면 기본값 반환)
	// ─────────────────────────────────────────────

	/**
	 * 시야 원점을 반환합니다.
	 * @param Object  ISightProvider 구현 UObject
	 * @param OutOrigin  결과 위치 (실패 시 FVector::ZeroVector)
	 * @return 성공 여부
	 */
	inline bool TryGetSightOrigin(const UObject* Object, FVector& OutOrigin)
	{
		if (const ISightProvider* Provider = TryGetProvider(Object))
		{
			OutOrigin = Provider->GetSightOrigin();
			return true;
		}
		OutOrigin = FVector::ZeroVector;
		return false;
	}

	/**
	 * 시야 범위를 반환합니다.
	 * @param Object  ISightProvider 구현 UObject
	 * @param OutRange  결과 범위 (실패 시 0.f)
	 * @return 성공 여부
	 */
	inline bool TryGetSightRange(const UObject* Object, float& OutRange)
	{
		if (const ISightProvider* Provider = TryGetProvider(Object))
		{
			OutRange = Provider->GetSightRange();
			return true;
		}
		OutRange = 0.f;
		return false;
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
	 * @param Object  ISightProvider 구현 UObject
	 * @param OutTeam  결과 팀 (실패 시 ERiftTeam::None)
	 * @return 성공 여부
	 */
	inline bool GetTeam(const UObject* Object, ERiftTeam& OutTeam)
	{
		if (const ISightProvider* Provider = TryGetProvider(Object))
		{
			OutTeam = Provider->GetTeam();
			return true;
		}
		OutTeam = ERiftTeam::None;
		return false;
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
	 * Observer의 시야 원점 기준으로 Target이 시야 범위 내에 있는지 확인합니다.
	 * 거리 계산은 XY 평면(2D) 기준으로 수행합니다.
	 * @param Observer  시야를 제공하는 UObject (ISightProvider)
	 * @param TargetLocation  확인할 월드 위치
	 * @return 시야 범위 내이면 true, Observer가 유효하지 않으면 false
	 */
	inline bool IsInSightRange(const UObject* Observer, const FVector& TargetLocation)
	{
		const ISightProvider* Provider = TryGetProvider(Observer);
		if (!Provider)
		{
			return false;
		}
		const float RangeSq = FMath::Square(Provider->GetSightRange());
		return FVector::DistSquared2D(Provider->GetSightOrigin(), TargetLocation) <= RangeSq;
	}

	/**
	 * Observer의 시야 원점 기준으로 Target UObject가 시야 범위 내에 있는지 확인합니다.
	 * Target도 ISightProvider를 구현해야 합니다.
	 * 거리 계산은 XY 평면(2D) 기준으로 수행합니다.
	 */
	inline bool IsInSightRange(const UObject* Observer, const UObject* Target)
	{
		FVector TargetOrigin;
		if (!GetSightOrigin(Target, TargetOrigin))
		{
			return false;
		}
		return IsInSightRange(Observer, TargetOrigin);
	}
} // namespace SightProviderHelper
