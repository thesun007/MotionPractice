// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/DJGameplayAbility.h"
#include "AbilitySystemComponent.h"

#include "Player/DJPlayerController.h"
#include "Character/DJCharacterBase.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DJGameplayAbility)

UDJGameplayAbility::UDJGameplayAbility(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;

	ActivationPolicy = EDJAbilityActivationPolicy::OnInputTriggered;
}

UDJAbilitySystemComponent* UDJGameplayAbility::GetDJAbilitySystemComponentFromActorInfo() const
{
	return Cast<UDJAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo_Checked());
}

ADJPlayerController* UDJGameplayAbility::GetDJPlayerControllerFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<ADJPlayerController>(CurrentActorInfo->PlayerController.Get()) : nullptr);
}

AController* UDJGameplayAbility::GetControllerFromActorInfo() const
{
	if (CurrentActorInfo)
	{
		if (AController* PC = CurrentActorInfo->PlayerController.Get())
		{
			return PC;
		}

		// Look for a player controller or pawn in the owner chain.
		AActor* TestActor = CurrentActorInfo->OwnerActor.Get();
		while (TestActor)
		{
			if (AController* C = Cast<AController>(TestActor))
			{
				return C;
			}

			if (APawn* Pawn = Cast<APawn>(TestActor))
			{
				return Pawn->GetController();
			}

			TestActor = TestActor->GetOwner();
		}
	}

	return nullptr;
}

ADJCharacterBase* UDJGameplayAbility::GetDJCharacterFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<ADJCharacterBase>(CurrentActorInfo->AvatarActor.Get()) : nullptr);
}

void UDJGameplayAbility::TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const
{
	const bool bIsPredicting = (Spec.ActivationInfo.ActivationMode == EGameplayAbilityActivationMode::Predicting);

	// Try to activate if activation policy is on spawn.
	if (ActorInfo && !Spec.IsActive() && !bIsPredicting && (ActivationPolicy == EDJAbilityActivationPolicy::OnSpawn))
	{
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		const AActor* AvatarActor = ActorInfo->AvatarActor.Get();

		// If avatar actor is torn off or about to die, don't try to activate until we get the new one.
		if (ASC && AvatarActor && !AvatarActor->GetTearOff() && (AvatarActor->GetLifeSpan() <= 0.0f))
		{
			const bool bIsLocalExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalPredicted) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalOnly);
			const bool bIsServerExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerOnly) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerInitiated);

			const bool bClientShouldActivate = ActorInfo->IsLocallyControlled() && bIsLocalExecution;
			const bool bServerShouldActivate = ActorInfo->IsNetAuthority() && bIsServerExecution;

			if (bClientShouldActivate || bServerShouldActivate)
			{
				ASC->TryActivateAbility(Spec.Handle);
			}
		}
	}
}

void UDJGameplayAbility::InitInputMapping()
{
	if (InputMapping.IsNull())	//입력 정보 없으면 그냥 통과
		return;

	ADJPlayerController* PC = GetDJPlayerControllerFromActorInfo();
	ADJCharacterBase* Character = GetDJCharacterFromActorInfo();
	if (PC == nullptr || Character == nullptr)
		return;

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		if ((InputComponent = Cast<UEnhancedInputComponent>(Character->InputComponent)) == nullptr)
			return;

		InputMapping.LoadSynchronous();
		if (UInputMappingContext* IMC = InputMapping.Get())
		{
			FModifyContextOptions Options = {};
			Options.bIgnoreAllPressedKeysUntilRelease = false;
			Subsystem->AddMappingContext(IMC, 1, Options);

			SetInputAction();
		}
	}
	else
		return ;
}

void UDJGameplayAbility::SetInputAction()
{
}

void UDJGameplayAbility::RemoveInputMapping()
{
	if (InputMapping == nullptr)	//입력 정보 없으면 그냥 통과
		return;

	ADJPlayerController* PC = GetDJPlayerControllerFromActorInfo();
	if (PC == nullptr)
		return;

	if (InputComponent)
	{
		if (InputHandles.IsEmpty())
			return;

		for (uint32 handle : InputHandles)
		{
			InputComponent->RemoveActionBindingForHandle(handle);
		}

	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		InputMapping.LoadSynchronous();
		if (UInputMappingContext* IMC = InputMapping.Get())
		{
			FModifyContextOptions Options = {};
			Options.bIgnoreAllPressedKeysUntilRelease = false;
			Subsystem->RemoveMappingContext(IMC, Options);
		}
	}

}

bool UDJGameplayAbility::CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags)
{
	bool result = Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags);
	if (!result)
		return result;

	InitInputMapping();
	return true;
}

void UDJGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	K2_OnAbilityAdded();

	TryActivateAbilityOnSpawn(ActorInfo, Spec);
}

void UDJGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	RemoveInputMapping();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
