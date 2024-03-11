// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GAS/DJAbilitySystemComponent.h"	//필수
#include "DJAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * Delegate used to broadcast attribute events, some of these parameters may be null on clients:
 * @param EffectInstigator	The original instigating actor for this event
 * @param EffectCauser		변화를 일으킨 물리적 행위자
 * @param EffectSpec		The full effect spec for this change
 * @param EffectMagnitude	The raw magnitude, this is before clamping
 * @param OldValue			The value of the attribute before it was changed
 * @param NewValue			The value after it was changed
*/
DECLARE_MULTICAST_DELEGATE_SixParams(FDJAttributeEvent, 
	AActor* /*EffectInstigator*/, AActor* /*EffectCauser*/, 
	const struct FGameplayEffectSpec* /*EffectSpec*/, float /*EffectMagnitude*/,
	float /*OldValue*/, float /*NewValue*/);

/**
 * 
 */
UCLASS()
class DJGAME_API UDJAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:

	UDJAbilitySystemComponent* GetLyraAbilitySystemComponent() const;
};
