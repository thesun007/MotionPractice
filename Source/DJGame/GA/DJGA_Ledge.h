// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GA/DJGameplayAbility.h"
#include "DJGA_Ledge.generated.h"

/**
 * 
 */
UCLASS()
class DJGAME_API UDJGA_Ledge : public UDJGameplayAbility
{
	GENERATED_BODY()
	
public:
	UDJGA_Ledge(const FObjectInitializer& ObjectInitializer);

protected:
	//~ GameAbility �Լ� ������
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	//~ ��

private:
	UPROPERTY()
	TObjectPtr<class ADJCharacterBase> Character;
	UPROPERTY()
	TObjectPtr<class UCharacterMovementComponent> Movement;
};
