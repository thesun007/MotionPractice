// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/DJInputComponent.h"
#include "EnhancedInputSubsystems.h"

UDJInputComponent::UDJInputComponent(const FObjectInitializer& ObjectInitializer)
{
}

void UDJInputComponent::AddInputMappings(const UDJInputData* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	//필요한 경우 InputConfig에서 무언가를 추가하기 위해, 사용자 정의 논리를 여기서 처리할 수 있습니다.
}

void UDJInputComponent::RemoveInputMappings(const UDJInputData* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);
}

void UDJInputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
	for (uint32 Handle : BindHandles)
	{
		RemoveBindingByHandle(Handle);
	}
	BindHandles.Reset();
}
