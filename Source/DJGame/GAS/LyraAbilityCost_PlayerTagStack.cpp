// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraAbilityCost_PlayerTagStack.h"

#include "GameFramework/Controller.h"
#include "GA/DJGameplayAbility.h"
#include "Player/DJPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraAbilityCost_PlayerTagStack)

ULyraAbilityCost_PlayerTagStack::ULyraAbilityCost_PlayerTagStack()
{
	Quantity.SetValue(1.0f);
}

bool ULyraAbilityCost_PlayerTagStack::CheckCost(const UDJGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (AController* PC = Ability->GetControllerFromActorInfo())
	{
		if (ADJPlayerState* PS = Cast<ADJPlayerState>(PC->PlayerState))
		{
			const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

			const float NumStacksReal = Quantity.GetValueAtLevel(AbilityLevel);
			const int32 NumStacks = FMath::TruncToInt(NumStacksReal);

			return PS->GetStatTagStackCount(Tag) >= NumStacks;
		}
	}
	return false;
}

void ULyraAbilityCost_PlayerTagStack::ApplyCost(const UDJGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo->IsNetAuthority())
	{
		if (AController* PC = Ability->GetControllerFromActorInfo())
		{
			if (ADJPlayerState* PS = Cast<ADJPlayerState>(PC->PlayerState))
			{
				const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

				const float NumStacksReal = Quantity.GetValueAtLevel(AbilityLevel);
				const int32 NumStacks = FMath::TruncToInt(NumStacksReal);

				PS->RemoveStatTagStack(Tag, NumStacks);
			}
		}
	}
}

