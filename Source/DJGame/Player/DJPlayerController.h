// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DJPlayerController.generated.h"



/**
 * 
 */
UCLASS()
class DJGAME_API ADJPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ADJPlayerController();

	UFUNCTION(BlueprintCallable, Category = "DJ|PlayerController")
	class ADJPlayerState* GetDJPlayerState() const;
	UFUNCTION(BlueprintCallable, Category = "DJ|PlayerController")
	class UDJAbilitySystemComponent* GetDJAbilitySystemComponent() const;

protected:
	void BeginPlay() override;
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused);
};
