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

	//커스텀 InputData를 추가로 인풋 로직에 추가
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
