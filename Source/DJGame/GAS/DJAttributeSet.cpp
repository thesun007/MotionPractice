// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/DJAttributeSet.h"
#include "DJAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DJAttributeSet)

UDJAbilitySystemComponent* UDJAttributeSet::GetLyraAbilitySystemComponent() const
{
	return Cast<UDJAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}
