// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/DJPlayerController.h"
#include "GameFramework/Character.h"
#include "GAS/DJAbilitySystemComponent.h"
#include "DJPlayerState.h"
ADJPlayerController::ADJPlayerController()
{
}

ADJPlayerState* ADJPlayerController::GetDJPlayerState() const
{
	return GetPlayerState<ADJPlayerState>();
}

UDJAbilitySystemComponent* ADJPlayerController::GetDJAbilitySystemComponent() const
{
	ADJPlayerState* DJPS = GetDJPlayerState();
	return (DJPS ? DJPS->GetDJAbilitySystemComponent() : nullptr);
}


void ADJPlayerController::BeginPlay()
{
	FInputModeGameOnly GameOnlyInputMode;
	SetInputMode(GameOnlyInputMode);
}

void ADJPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (UDJAbilitySystemComponent* DJASC = GetDJAbilitySystemComponent())
	{
		DJASC->ProcessAbilityInput(DeltaTime, bGamePaused);
	}

	Super::PostProcessInput(DeltaTime, bGamePaused);
}
