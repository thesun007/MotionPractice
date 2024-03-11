// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/DJGA_AssassinationVictim.h"
#include "Character/DJCharacterBase.h"
#include "MotionWarpingComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GA/DJGA_Assassination.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_InputTag_AssassinationVictim, "InputTag.Ability.AssassinationVictim");
UE_DEFINE_GAMEPLAY_TAG(TAG_AbilityType_Action_AssassinationVictim, "Ability.Type.Action.AssassinationVictim");

UDJGA_AssassinationVictim::UDJGA_AssassinationVictim(const FObjectInitializer& ObjectInitializer)
{
	AbilityTags.AddTag(TAG_AbilityType_Action_AssassinationVictim);
	FAbilityTriggerData trigger;
	trigger.TriggerTag = TAG_InputTag_AssassinationVictim;
	AbilityTriggers.Add(trigger);
}

void UDJGA_AssassinationVictim::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ADJCharacterBase* Instigator = Cast<ADJCharacterBase>(TriggerEventData->Instigator);
	Character = GetDJCharacterFromActorInfo();
	if (Instigator && Character)
	{
		//임시
		/*FVector InstigatorToVictimDir = Character->GetActorLocation() - Instigator->GetActorLocation();
		InstigatorToVictimDir.Z = 0;	*/	

		//몽타주 없으면 취소
		if (AssassinationVictimMontages.IsEmpty())
		{
			CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
			return;
		}

		//선동 어빌리티가 암살 어빌리티가 아니면 취소
		const UDJGA_Assassination* InstigatorAbility = Cast<UDJGA_Assassination>(TriggerEventData->ContextHandle.GetAbilityInstance_NotReplicated());
		if (InstigatorAbility == nullptr)
		{
			CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
			return;
		}

		/* 모션와핑 적용 */
		Character->SetActorEnableCollision(false);	//루트모션 비활성화 됨
		Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);	//루트모션 다시 활성화 됨
		/*UMotionWarpingComponent* MWC = Character->GetMotionWarpingComponent();
		if (MWC)
		{
			MWC->AddOrUpdateWarpTargetFromLocationAndRotation(FName(TEXT("StartPos")), FVector::ZeroVector, FRotationMatrix::MakeFromX(InstigatorToVictimDir).Rotator());
		}*/

		uint32 index = InstigatorAbility->GetMontageIndex();
		UAbilityTask_PlayMontageAndWait* AssassinationMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("AssassinationVictim"), AssassinationVictimMontages[index], 1.f, NAME_None);

		Character->GetMesh()->GetAnimInstance()->OnPlayMontageNotifyBegin.AddDynamic(this, &ThisClass::MontageBeginNotifyCallback);
		AssassinationMontage->OnInterrupted.AddDynamic(this, &ThisClass::EndFunction);
		AssassinationMontage->OnCancelled.AddDynamic(this, &ThisClass::EndFunction);
		AssassinationMontage->OnCompleted.AddDynamic(this, &ThisClass::EndFunction);
		AssassinationMontage->ReadyForActivation();
	}

}

void UDJGA_AssassinationVictim::EndFunction()
{
	Character->GetMesh()->GetAnimInstance()->OnPlayMontageNotifyBegin.RemoveDynamic(this, &ThisClass::MontageBeginNotifyCallback);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UDJGA_AssassinationVictim::MontageBeginNotifyCallback(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	//죽음
	Character->GetCharacterMovement()->DisableMovement();
	//Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	Character->GetMesh()->SetSimulatePhysics(true);
	Character->SetActorEnableCollision(true);
	Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}
