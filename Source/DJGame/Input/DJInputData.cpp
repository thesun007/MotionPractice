// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/DJInputData.h"
#include "DJGame.h"

const UInputAction* UDJInputData::FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const auto& NativeInput : NativeInputActions)
	{
		if (NativeInput.InputAction && (NativeInput.InputTag == InputTag))
		{
			return NativeInput.InputAction;
		}
	}

	if (bLogNotFound)
	{
		DJ_LOG(DJLog, Error, TEXT("Can't find NativeInputAction for InputTag [%s] on InputConfig."), *InputTag.ToString())
	}
	return nullptr;
}

const UInputAction* UDJInputData::FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const auto& Action : AbilityInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		DJ_LOG(DJLog, Error, TEXT("Can't find AbilityInputAction for InputTag [%s] on InputConfig."), *InputTag.ToString());
	}

	return nullptr;
}
