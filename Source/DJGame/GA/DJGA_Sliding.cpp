// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/DJGA_Sliding.h"
#include "Tag/DJGameplayTags.h"
#include "DJGame.h"
#include "Character/DJCharacterBase.h"
#include "Character/DJCharacterMovementComponent.h"
#include "Abilities\Tasks\AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "AT/DJAT_Trigger.h"
#include "GA/TA/TargetActor_TickableRadius.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AT/DJAT_TickBind.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_InputTag_Sliding, "InputTag.Ability.Sliding");
UE_DEFINE_GAMEPLAY_TAG(TAG_AbilityType_Action_Sliding, "Ability.Type.Action.Sliding");

UDJGA_Sliding::UDJGA_Sliding(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	AbilityTags.AddTag(TAG_AbilityType_Action_Sliding);
	FAbilityTriggerData trigger;
	trigger.TriggerTag = TAG_InputTag_Sliding;
	AbilityTriggers.Add(trigger);

	ActivationOwnedTags.AddTag(Tag_Status_Sliding);
	//ActivationBlockedTags.AddTag(Tag_Status_Sliding);

	Deceleration = 500.f;
	MaxFloorAngle = 35.f;
	Duration = 0.5f;
}

void UDJGA_Sliding::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (IsLocallyControlled())
	{
		/* �ʱ�ȭ �� ���� üũ */
		Character = GetDJCharacterFromActorInfo();
		if (Character == nullptr)
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}
		Movement = Character->GetCharacterMovement();
		if (Movement == nullptr)
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}
		IsEnding = false;

		//�̵� ����
		FVector VelocityDir = Movement->Velocity;	VelocityDir.Normalize();
		float CurrentSpeed = Movement->Velocity.Length()+100;
		FVector LastInputDir = Movement->GetLastInputVector();
		FVector ForwardDir = Character->GetActorForwardVector();

		float DesireAngle = VelocityDir.Dot(LastInputDir);	// [�ӵ������ �Է¹�����] 0.707(�� 45��) ���Ͽ��� ��
		float MoveAngle = ForwardDir.Dot(LastInputDir);			// [ĳ���� ����� �Է¹�����] 0.707(�� 45��) ���Ͽ��� ��

		// ������, ĳ���Ͱ� �ƴϰų� �ӵ��� ������ ���
		if (Movement->IsFalling() || CurrentSpeed < 540 ||
			DesireAngle <0.707 || MoveAngle <0.707)
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}
		
		/* �Ÿ�,��ġ ��� */
		//������ ������ ���
		if (Deceleration <= 0)
		{
			DJ_LOG(DJLog, Log, TEXT("[Sliding false] Deceleration is Zero or Less."));
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}
		//float VelocityZeroTime = CurrentSpeed / Deceleration;	//�ӵ��� 0�Ǳ���� �ð�
		//float Distance = CurrentSpeed * VelocityZeroTime - (Deceleration * VelocityZeroTime * VelocityZeroTime) * 0.5f;	//�̵��� �� �Ÿ�

		////���� ������
		//FVector Destination = Character->GetActorLocation() + LastInputDir * Distance;
		
		
		//** ���� ���� ����� ������ ������, [������/������ �߰� ���, ���� �� ���� ����] **//
		FVector GroundNormal = Movement->CurrentFloor.HitResult.ImpactNormal;
		float CurrentFloorAngle = FRotationMatrix::MakeFromZX(GroundNormal, LastInputDir).Rotator().Pitch;	//��������: ���̳ʽ� ����		/*FMath::RadiansToDegrees(FMath::Acos(GroundNormal.Dot(Character->GetActorUpVector())));*/
		//���� ������ MaxAngle���� ���ĸ��� ���
		if (MaxFloorAngle < FMath::Abs(CurrentFloorAngle) )
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}
		float Magnitude = CurrentFloorAngle / MaxFloorAngle ;					//(-0.5 ~ 0.5) (�������� ~ ��������)
		//**���� ������**//
		FinalDeceleration = Deceleration + (Magnitude * Deceleration);				
		FVector FinalDir = GroundNormal.Cross(LastInputDir).Cross(GroundNormal);	//<���������� ������ ���� ���� ����> ������ ���� ���⿡ ���� ������ ���͸� ���ϰ�, �ٽ� ������ ����


		//** ���� ���� ���� **//
		TargetActorRadius = GetWorld()->SpawnActor<ATargetActor_TickableRadius>(CurrentActorInfo->AvatarActor.Get()->GetActorLocation(),
			CurrentActorInfo->AvatarActor.Get()->GetActorRotation());

		//���� ����
		TargetActorRadius->Radius = 15;
		TargetActorRadius->bDebug = bDebug;
		TargetActorRadius->Offset = FVector(0.0, 0.0, 30.0);	// �� ��ġ���� ���� 30��ġ, ������ 15 ��

		FGameplayAbilityTargetingLocationInfo LocationInfo = MakeTargetLocationInfoFromOwnerSkeletalMeshComponent(FName("foot_r"));
		LocationInfo.SourceActor = GetAvatarActorFromActorInfo();
		LocationInfo.SourceAbility = this;
		TargetActorRadius->StartLocation = LocationInfo;	//���� ������ ��ġ�� ������ġ�� ����

		//���� ����
		FGameplayTargetDataFilterHandle filterHandle;
		FGameplayTargetDataFilter filter;
		filter.SelfActor = CurrentActorInfo->AvatarActor.Get();
		filter.RequiredActorClass = AActor::StaticClass();
		filter.SelfFilter = ETargetDataFilterSelf::TDFS_NoSelf;
		filterHandle.Filter = MakeShared<FGameplayTargetDataFilter, ESPMode::ThreadSafe>(filter);
		TargetActorRadius->Filter = filterHandle;

		//(���� ��ֹ� Ȯ�ο�)Ʈ���� �½�ũ ����
		UDJAT_Trigger* TriggerTask = UDJAT_Trigger::CreateTriggerTaskUsingActor(this, TEXT("SlidingTask"), EGameplayTargetingConfirmation::Instant, false, TargetActorRadius);	//TargetActor�� Trigger �½�ũ �� �Բ� �Ҹ�
		TriggerTask->ValidData.AddDynamic(this, &ThisClass::OnGetTargetCallback);
		TriggerTask->ReadyForActivation();
		
		/* Ŀ�� �õ� */
		bool commitResult = CommitAbility(Handle, ActorInfo, ActivationInfo);
		if (commitResult == false)
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}

		////Ŭ���̾�Ʈ�� ������ ���� ����
		//if (!HasAuthority(&ActivationInfo))
		//{
		//	SendInfo(FinalDir, Montage);
		//}
		// 
		
		//* ��Ÿ�� ���� �� �̵� Task ���� *//
		ProcessMontageNTask(FinalDir, CurrentSpeed);
	}
}

void UDJGA_Sliding::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if(TargetActorRadius)
		TargetActorRadius->Destroy();

	if (RotateTimerHandle.IsValid())
		Character->GetWorld()->GetTimerManager().ClearTimer(RotateTimerHandle);
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDJGA_Sliding::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	//Super::InputPressed(Handle, ActorInfo, ActivationInfo);

	//�ٽ� ������ ��, ����
	bEndCommand = true;

}

void UDJGA_Sliding::EndFunction()
{
	Character->GetMesh()->SetRelativeRotation(FRotator(0.0, -90.0, 0.0).Quaternion());
	Movement->BrakingDecelerationWalking = OriginalDeceleration;
	Character->GetCharacterMovement()->bUseControllerDesiredRotation = true;
	Character->GetMesh()->GetAnimInstance()->OnPlayMontageNotifyBegin.RemoveDynamic(this, &ThisClass::MontageBeginNotifyCallback);

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UDJGA_Sliding::StartEnding()
{
	Movement->RemoveRootMotionSource(FName("SlidingMove"));
	MontageJumpToSection("End");
	
	IsEnding = true;
}

void UDJGA_Sliding::CheckUpper()
{
	//�Ͼ ��, crouch? or uncrouch
	FVector Pos = Character->GetActorLocation() + FVector(0.0, 0.0, Character->GetSimpleCollisionHalfHeight() * 2 - Movement->CrouchedHalfHeight);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(RadiusTargetingOverlap), false, Character);
	Params.bReturnPhysicalMaterial = false;

	bool bOverlap = Character->GetWorld()->OverlapAnyTestByObjectType(Pos, FQuat::Identity, FCollisionObjectQueryParams(ECC_WorldStatic), FCollisionShape::MakeSphere(15.f), Params);
	
	if (bOverlap)
	{
		Character->Crouch();
		EndFunction();
	}
	else
		Character->UnCrouch();

	Movement->BrakingDecelerationWalking = OriginalDeceleration;
}

void UDJGA_Sliding::OnGetTargetCallback(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	//�տ� ��ֹ� ���� ��, �����̵� ����.
	for (int i = 0; i < TargetDataHandle.Num(); ++i)
	{
		if (UAbilitySystemBlueprintLibrary::TargetDataHasHitResult(TargetDataHandle, i))
		{
			FHitResult result = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(TargetDataHandle, i);

			float ImpactNormalAngle = FMath::RadiansToDegrees(FMath::Acos(result.ImpactNormal.Dot(Character->GetMesh()->GetUpVector() )));
			if (MaxFloorAngle < ImpactNormalAngle)	//�����̵� �Ұ����� ���ĸ� ���� ���� (�ٸ����� ���ع��� �Ǵ� ��)
			{
				if (IsEnding == false)
				{
					Movement->Velocity = Movement->Velocity * 0.5f;
					StartEnding();
					break;
				}
			}
		}
	}
}

void UDJGA_Sliding::Tick_ProcessOnGround(float DeltaTime)
{
	if (Movement->IsMovingOnGround())
	{
		FRotator Target = FRotator(0.f,-90.f,0.f);
		if (IsEnding == false)
		{
			//�����̵� ��, ���� ������ ���� �ڼ� ��Ī
			Target = FRotationMatrix::MakeFromZX(Movement->CurrentFloor.HitResult.ImpactNormal, Character->GetMesh()->GetForwardVector()).Rotator();
			Character->GetMesh()->SetWorldRotation(FMath::RInterpConstantTo(Character->GetMesh()->GetComponentRotation(), Target, DeltaTime, 80.f).Quaternion());
		}
		else
		{
			//������ ��� ��, ������� �ѹ�
			Character->GetMesh()->SetRelativeRotation(FMath::RInterpConstantTo(Character->GetMesh()->GetRelativeRotation(), Target, DeltaTime, 80.f).Quaternion());
		}
			
		//������� ��
		FName SectionName = Character->GetMesh()->GetAnimInstance()->Montage_GetCurrentSection(SlidingMontage);
		float CurrentSpeed = Movement->Velocity.Length();
		if (SectionName == FName("Loop") && (bEndCommand || CurrentSpeed < 500) )	//�Ǵ� �ӵ��� ������ŭ �������� ������ �������� ��ȯ
		{ 
			if (IsEnding == false)
			{
				StartEnding();
			}
			bEndCommand = false;
		}
	}
	else
	{
		Character->UnCrouch();
		EndFunction();	// ���߿� ���� �� �ٷ� ����
	}
}

void UDJGA_Sliding::ProcessMontageNTask(FVector Dir, float Strength)
{
	//���� ��, ���� ����
	float Yaw = FRotationMatrix::MakeFromX(Dir).Rotator().Yaw;
	FRotator meshRotator = Character->GetMesh()->GetComponentRotation();
	meshRotator.Yaw = Yaw - 90.0;
	Character->GetMesh()->SetWorldRotation(meshRotator);	//���� �������� ĳ���� ȸ��
	Character->GetCharacterMovement()->bUseControllerDesiredRotation = false;

	//**��Ÿ�� ���� �⺻ �����Ƽ �׽�ũ ���**//
	UAbilityTask_PlayMontageAndWait* AT_SlidingMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("Sliding"),
		SlidingMontage, 1.f);

	Character->GetMesh()->GetAnimInstance()->OnPlayMontageNotifyBegin.AddDynamic(this, &ThisClass::MontageBeginNotifyCallback);
	AT_SlidingMontage->OnInterrupted.AddDynamic(this, &ThisClass::EndFunction);
	AT_SlidingMontage->OnCancelled.AddDynamic(this, &ThisClass::EndFunction);
	AT_SlidingMontage->OnCompleted.AddDynamic(this, &ThisClass::EndFunction);
	AT_SlidingMontage->ReadyForActivation();
	
	//�̵� �½�ũ ����
	OriginalDeceleration = Movement->BrakingDecelerationWalking;
	Movement->BrakingDecelerationWalking = FinalDeceleration;
	UAbilityTask_ApplyRootMotionConstantForce* AT_SlidingMove = UAbilityTask_ApplyRootMotionConstantForce::
		ApplyRootMotionConstantForce(this, FName("SlidingMove"), Dir, Strength, Duration, false, nullptr, 
			ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity, FVector::ZeroVector, 0, true);
	
	//AT_SlidingMove->OnFinish.AddDynamic(this, &ThisClass::EndFunction);
	AT_SlidingMove->ReadyForActivation();

	/** ���� ������ ���� �ڼ� ��Ī ���� **/
	UDJAT_TickBind* TickTask = UDJAT_TickBind::CreateTickBindTask(this, FName("OnGround"));
	check(TickTask);
	TickTask->OnTick.AddDynamic(this, &ThisClass::Tick_ProcessOnGround);
	TickTask->ReadyForActivation();
}

void UDJGA_Sliding::MontageBeginNotifyCallback(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if (NotifyName == FName("CheckUpper"))
	{
		CheckUpper();
	}
}
