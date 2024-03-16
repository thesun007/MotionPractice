// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Abilities/GameplayAbilityTargetTypes.h"

#include "LyraGameplayAbilityTargetData_SingleTargetHit.generated.h"

class FArchive;
struct FGameplayEffectContextHandle;


/** Game-specific additions to SingleTargetHit tracking */
USTRUCT()
struct FLyraGameplayAbilityTargetData_SingleTargetHit : public FGameplayAbilityTargetData_SingleTargetHit
{
	GENERATED_BODY()

	FLyraGameplayAbilityTargetData_SingleTargetHit()
		: CartridgeID(-1)
	{ }

	virtual void AddTargetDataToContext(FGameplayEffectContextHandle& Context, bool bIncludeActorArray) const override;

	/** 동일한 카트리지에 포함된 여러 총알을 식별할 수 있는 ID */
	UPROPERTY()
	int32 CartridgeID;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FLyraGameplayAbilityTargetData_SingleTargetHit::StaticStruct();
	}
};

template<>
struct TStructOpsTypeTraits<FLyraGameplayAbilityTargetData_SingleTargetHit> : public TStructOpsTypeTraitsBase2<FLyraGameplayAbilityTargetData_SingleTargetHit>
{
	enum
	{
		WithNetSerializer = true	// For now this is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};

