// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/DJGA_Ledge.h"
#include "Tag/DJGameplayTags.h"
#include "Character/DJCharacterBase.h"
#include "Character/DJCharacterMovementComponent.h"
#include "Player/DJPlayerController.h"

#include "Abilities\Tasks\AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"

#include "Input/DJInputComponent.h"
#include "InputActionValue.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_InputTag_Ledge, "InputTag.Ability.Ledge");
UE_DEFINE_GAMEPLAY_TAG(TAG_AbilityType_Action_Ledge, "Ability.Type.Action.Ledge");

UDJGA_Ledge::UDJGA_Ledge(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	//태그 초기화
	AbilityTags.AddTag(TAG_AbilityType_Action_Ledge);
	FAbilityTriggerData trigger;
	trigger.TriggerTag = TAG_InputTag_Ledge;
	AbilityTriggers.Add(trigger);
	ActivationOwnedTags.AddTag(Tag_Status_Ledge);
}

void UDJGA_Ledge::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (IsLocallyControlled() == false)
		return;
	

	//기본 필요 자원 확보


	//ledge 찾기
	
	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UDJGA_Ledge::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDJGA_Ledge::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
}

void UDJGA_Ledge::SetInputAction()
{	
	BindNativeAction(InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Move, false);
}

void UDJGA_Ledge::Move(const FInputActionValue& InputActionValue)
{
	
}
