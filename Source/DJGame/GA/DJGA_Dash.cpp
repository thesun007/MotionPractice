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
		// Dir 초기화
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

		//움직이지 않았다면 대시 취소
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

		//클라이언트면 서버로 정보 전송
		if (!HasAuthority(&ActivationInfo))
		{
			SendInfo(FinalDir, Montage);
		}

		//몽타주 실행 및 이동 Task 실행
		ProcessMontageNTask(FinalDir, Montage);

		//게임플레이 큐 실행

		//위젯 메시지 (쿨타임 남은시간 전송)
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
	//몽타주 실행 기본 어빌리티 테스크 사용
	//Rate와 시작 섹션 이름 추가 지정
	UAbilityTask_PlayMontageAndWait* AT_DashMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("Dash"),
		_Montage, 1.f);

	AT_DashMontage->OnInterrupted.AddDynamic(this, &UDJGA_Dash::EndFunction);
	AT_DashMontage->OnCancelled.AddDynamic(this, &UDJGA_Dash::EndFunction);

	AT_DashMontage->ReadyForActivation();

	//루트모션 이동
	UAbilityTask_ApplyRootMotionConstantForce* AT_ApplyDash = UAbilityTask_ApplyRootMotionConstantForce::
		ApplyRootMotionConstantForce(this,FName(),_Dir, Strength, Duration, true, 
			nullptr, ERootMotionFinishVelocityMode::ClampVelocity,
			FVector::ZeroVector, 1000, true);
	
	AT_ApplyDash->OnFinish.AddDynamic(this, &UDJGA_Dash::OnTaskFinishCallback);
	AT_ApplyDash->ReadyForActivation();
}
