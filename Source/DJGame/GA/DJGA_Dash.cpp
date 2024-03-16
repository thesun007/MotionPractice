// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/DJGA_Dash.h"
#include "Player/DJPlayerController.h"
#include "Character/DJCharacterBase.h"
#include "Camera/DJCameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities\Tasks\AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "System/LyraInteractionDurationMessage.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Tag/DJGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_InputTag_Dash, "InputTag.Ability.Dash");
UE_DEFINE_GAMEPLAY_TAG(TAG_AbilityType_Action_Dash, "Ability.Type.Action.Dash");

UDJGA_Dash::UDJGA_Dash(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer),
	Strength(1850), Duration(0.25)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	AbilityTags.AddTag(TAG_AbilityType_Action_Dash);
	FAbilityTriggerData trigger;
	trigger.TriggerTag = TAG_InputTag_Dash;
	AbilityTriggers.Add(trigger);

	ActivationOwnedTags.AddTag(Tag_Status_Dashing);
	ActivationBlockedTags.AddTag(Tag_Status_Assassinationing);
}

void UDJGA_Dash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (IsLocallyControlled())
	{
		// Dir �ʱ�ȭ
		FVector FaceDir = FVector::ZeroVector;
		FVector MoveDir = FVector::ZeroVector;
		FVector CamDir = FVector::ZeroVector;

		ADJCharacterBase* Character = GetDJCharacterFromActorInfo();
		if (!Character)
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}

		//Get Dir
		FaceDir = Character->GetActorForwardVector();
		MoveDir = Character->GetLastMovementInputVector();
		MoveDir.Normalize();
		if (UDJCameraComponent* Cam = GetDJCharacterFromActorInfo()->GetComponentByClass<UDJCameraComponent>())
		{
			CamDir = Cam->GetForwardVector() * FVector(1.0, 1.0, 1.5);	//Help add up dir
		}

		//�������� �ʾҴٸ� ��� ���
		if (MoveDir.IsNearlyZero())
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}
		bool commitResult = CommitAbility(Handle, ActorInfo, ActivationInfo);
		if (commitResult == false)
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}

		//set Montage and final Dir
		float Angle = FMath::RadiansToDegrees(FMath::Acos(FaceDir.Dot(MoveDir)));
		float LeftRight = FaceDir.Cross(MoveDir).Dot(FVector::UpVector);
		bool bMoveForward = false;
		FVector FinalDir = CamDir;

		if (Angle < 45)
		{
			bMoveForward = true;
			Montage = DashMontage.Forward;
		}
		else if (Angle > 135)
			Montage = DashMontage.Backward;
		else
		{
			if (LeftRight < 0)
				Montage = DashMontage.Left;
			else
				Montage = DashMontage.Right;
		}

		if (!bMoveForward)
			FinalDir = MoveDir;

		//UnCrouch
		if (Character->GetMovementComponent()->IsCrouching())
		{
			Character->UnCrouch();
		}

		//Ŭ���̾�Ʈ�� ������ ���� ����
		if (!HasAuthority(&ActivationInfo))
		{
			SendInfo(FinalDir, Montage);
		}

		//��Ÿ�� ���� �� �̵� Task ����
		ProcessMontageNTask(FinalDir, Montage);

		//�����÷��� ť ����

		//���� �޽��� (��Ÿ�� �����ð� ����)
		UGameInstance* GameInstance = GetWorld()->GetGameInstance();
		UGameplayMessageSubsystem* MySubsystem = GameInstance->GetSubsystem<UGameplayMessageSubsystem>();
		//UGameplayMessageSubsystem::Get(GetWorld());
		FLyraInteractionDurationMessage message;
		message.Instigator = GetDJCharacterFromActorInfo();
		message.Duration = GetCooldownTimeRemaining();
		MySubsystem->BroadcastMessage(DASH_DURATION_MESSAGE, message);
	}

}

void UDJGA_Dash::EndFunction()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UDJGA_Dash::OnTaskFinishCallback()
{
	UE_LOG(LogTemp, Log, TEXT("Dash Onfinish!"));
	FTimerHandle finish;
	GetWorld()->GetTimerManager().SetTimer(finish, this, &UDJGA_Dash::EndFunction,
		0.5f, false);
}

void UDJGA_Dash::SendInfo_Implementation(FVector _Dir, UAnimMontage* _Montage)
{
	ProcessMontageNTask(_Dir, _Montage);
}

void UDJGA_Dash::ProcessMontageNTask(FVector _Dir, UAnimMontage* _Montage)
{
	//��Ÿ�� ���� �⺻ �����Ƽ �׽�ũ ���
	//Rate�� ���� ���� �̸� �߰� ����
	UAbilityTask_PlayMontageAndWait* AT_DashMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("Dash"),
		_Montage, 1.f);

	AT_DashMontage->OnInterrupted.AddDynamic(this, &UDJGA_Dash::EndFunction);
	AT_DashMontage->OnCancelled.AddDynamic(this, &UDJGA_Dash::EndFunction);

	AT_DashMontage->ReadyForActivation();

	//��Ʈ��� �̵�
	UAbilityTask_ApplyRootMotionConstantForce* AT_ApplyDash = UAbilityTask_ApplyRootMotionConstantForce::
		ApplyRootMotionConstantForce(this,FName(),_Dir, Strength, Duration, true, 
			nullptr, ERootMotionFinishVelocityMode::ClampVelocity,
			FVector::ZeroVector, 1000, true);
	
	AT_ApplyDash->OnFinish.AddDynamic(this, &UDJGA_Dash::OnTaskFinishCallback);
	AT_ApplyDash->ReadyForActivation();
}
