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
		/* 초기화 및 조건 체크 */
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

		//이동 정보
		FVector VelocityDir = Movement->Velocity;	VelocityDir.Normalize();
		float CurrentSpeed = Movement->Velocity.Length()+100;
		FVector LastInputDir = Movement->GetLastInputVector();
		FVector ForwardDir = Character->GetActorForwardVector();

		float DesireAngle = VelocityDir.Dot(LastInputDir);	// [속도방향과 입력방향차] 0.707(약 45도) 이하여야 함
		float MoveAngle = ForwardDir.Dot(LastInputDir);			// [캐릭터 정면과 입력방향차] 0.707(약 45도) 이하여야 함

		// 낙하중, 캐릭터가 아니거나 속도가 낮으면 취소
		if (Movement->IsFalling() || CurrentSpeed < 540 ||
			DesireAngle <0.707 || MoveAngle <0.707)
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}
		
		/* 거리,위치 계산 */
		//감쇠계수 없으면 취소
		if (Deceleration <= 0)
		{
			DJ_LOG(DJLog, Log, TEXT("[Sliding false] Deceleration is Zero or Less."));
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}
		//float VelocityZeroTime = CurrentSpeed / Deceleration;	//속도가 0되기까지 시간
		//float Distance = CurrentSpeed * VelocityZeroTime - (Deceleration * VelocityZeroTime * VelocityZeroTime) * 0.5f;	//이동할 총 거리

		////최종 목적지
		//FVector Destination = Character->GetActorLocation() + LastInputDir * Distance;
		
		
		//** 현재 지면 기울기와 정면의 각도차, [오르막/내리막 추가 계수, 최종 힘 방향 구함] **//
		FVector GroundNormal = Movement->CurrentFloor.HitResult.ImpactNormal;
		float CurrentFloorAngle = FRotationMatrix::MakeFromZX(GroundNormal, LastInputDir).Rotator().Pitch;	//내리막길: 마이너스 각도		/*FMath::RadiansToDegrees(FMath::Acos(GroundNormal.Dot(Character->GetActorUpVector())));*/
		//현재 지면이 MaxAngle보다 가파르면 취소
		if (MaxFloorAngle < FMath::Abs(CurrentFloorAngle) )
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}
		float Magnitude = CurrentFloorAngle / MaxFloorAngle ;					//(-0.5 ~ 0.5) (내리막길 ~ 오르막길)
		//**최종 감쇠계수**//
		FinalDeceleration = Deceleration + (Magnitude * Deceleration);				
		FVector FinalDir = GroundNormal.Cross(LastInputDir).Cross(GroundNormal);	//<기울기쪽으로 변형된 최종 진행 방향> 법선과 진행 방향에 대한 오른쪽 벡터를 구하고, 다시 법선과 외적


		//** 정면 감시 시작 **//
		TargetActorRadius = GetWorld()->SpawnActor<ATargetActor_TickableRadius>(CurrentActorInfo->AvatarActor.Get()->GetActorLocation(),
			CurrentActorInfo->AvatarActor.Get()->GetActorRotation());

		//라인 세팅
		TargetActorRadius->Radius = 15;
		TargetActorRadius->bDebug = bDebug;
		TargetActorRadius->Offset = FVector(0.0, 0.0, 30.0);	// 발 위치에서 위로 30센치, 반지름 15 구

		FGameplayAbilityTargetingLocationInfo LocationInfo = MakeTargetLocationInfoFromOwnerSkeletalMeshComponent(FName("foot_r"));
		LocationInfo.SourceActor = GetAvatarActorFromActorInfo();
		LocationInfo.SourceAbility = this;
		TargetActorRadius->StartLocation = LocationInfo;	//오너 액터의 위치를 시작위치로 설정

		//필터 적용
		FGameplayTargetDataFilterHandle filterHandle;
		FGameplayTargetDataFilter filter;
		filter.SelfActor = CurrentActorInfo->AvatarActor.Get();
		filter.RequiredActorClass = AActor::StaticClass();
		filter.SelfFilter = ETargetDataFilterSelf::TDFS_NoSelf;
		filterHandle.Filter = MakeShared<FGameplayTargetDataFilter, ESPMode::ThreadSafe>(filter);
		TargetActorRadius->Filter = filterHandle;

		//(정면 장애물 확인용)트리거 태스크 시작
		UDJAT_Trigger* TriggerTask = UDJAT_Trigger::CreateTriggerTaskUsingActor(this, TEXT("SlidingTask"), EGameplayTargetingConfirmation::Instant, false, TargetActorRadius);	//TargetActor는 Trigger 태스크 와 함께 소멸
		TriggerTask->ValidData.AddDynamic(this, &ThisClass::OnGetTargetCallback);
		TriggerTask->ReadyForActivation();
		
		/* 커밋 시도 */
		bool commitResult = CommitAbility(Handle, ActorInfo, ActivationInfo);
		if (commitResult == false)
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}

		////클라이언트면 서버로 정보 전송
		//if (!HasAuthority(&ActivationInfo))
		//{
		//	SendInfo(FinalDir, Montage);
		//}
		// 
		
		//* 몽타주 실행 및 이동 Task 실행 *//
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

	//다시 눌렀을 때, 종료
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
	//일어날 때, crouch? or uncrouch
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
	//앞에 장애물 있을 시, 슬라이딩 멈춤.
	for (int i = 0; i < TargetDataHandle.Num(); ++i)
	{
		if (UAbilitySystemBlueprintLibrary::TargetDataHasHitResult(TargetDataHandle, i))
		{
			FHitResult result = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(TargetDataHandle, i);

			float ImpactNormalAngle = FMath::RadiansToDegrees(FMath::Acos(result.ImpactNormal.Dot(Character->GetMesh()->GetUpVector() )));
			if (MaxFloorAngle < ImpactNormalAngle)	//슬라이딩 불가능한 가파른 지면 각도 (다른말로 방해물이 되는 벽)
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
			//슬라이딩 중, 지면 각도에 따라 자세 매칭
			Target = FRotationMatrix::MakeFromZX(Movement->CurrentFloor.HitResult.ImpactNormal, Character->GetMesh()->GetForwardVector()).Rotator();
			Character->GetMesh()->SetWorldRotation(FMath::RInterpConstantTo(Character->GetMesh()->GetComponentRotation(), Target, DeltaTime, 80.f).Quaternion());
		}
		else
		{
			//마무리 모션 중, 원래대로 롤백
			Character->GetMesh()->SetRelativeRotation(FMath::RInterpConstantTo(Character->GetMesh()->GetRelativeRotation(), Target, DeltaTime, 80.f).Quaternion());
		}
			
		//취소했을 떄
		FName SectionName = Character->GetMesh()->GetAnimInstance()->Montage_GetCurrentSection(SlidingMontage);
		float CurrentSpeed = Movement->Velocity.Length();
		if (SectionName == FName("Loop") && (bEndCommand || CurrentSpeed < 500) )	//또는 속도가 일정만큼 낮아지면 마무리 동작으로 전환
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
		EndFunction();	// 공중에 있을 땐 바로 종료
	}
}

void UDJGA_Sliding::ProcessMontageNTask(FVector Dir, float Strength)
{
	//시작 전, 사전 설정
	float Yaw = FRotationMatrix::MakeFromX(Dir).Rotator().Yaw;
	FRotator meshRotator = Character->GetMesh()->GetComponentRotation();
	meshRotator.Yaw = Yaw - 90.0;
	Character->GetMesh()->SetWorldRotation(meshRotator);	//진행 방향으로 캐릭터 회전
	Character->GetCharacterMovement()->bUseControllerDesiredRotation = false;

	//**몽타주 실행 기본 어빌리티 테스크 사용**//
	UAbilityTask_PlayMontageAndWait* AT_SlidingMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("Sliding"),
		SlidingMontage, 1.f);

	Character->GetMesh()->GetAnimInstance()->OnPlayMontageNotifyBegin.AddDynamic(this, &ThisClass::MontageBeginNotifyCallback);
	AT_SlidingMontage->OnInterrupted.AddDynamic(this, &ThisClass::EndFunction);
	AT_SlidingMontage->OnCancelled.AddDynamic(this, &ThisClass::EndFunction);
	AT_SlidingMontage->OnCompleted.AddDynamic(this, &ThisClass::EndFunction);
	AT_SlidingMontage->ReadyForActivation();
	
	//이동 태스크 시작
	OriginalDeceleration = Movement->BrakingDecelerationWalking;
	Movement->BrakingDecelerationWalking = FinalDeceleration;
	UAbilityTask_ApplyRootMotionConstantForce* AT_SlidingMove = UAbilityTask_ApplyRootMotionConstantForce::
		ApplyRootMotionConstantForce(this, FName("SlidingMove"), Dir, Strength, Duration, false, nullptr, 
			ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity, FVector::ZeroVector, 0, true);
	
	//AT_SlidingMove->OnFinish.AddDynamic(this, &ThisClass::EndFunction);
	AT_SlidingMove->ReadyForActivation();

	/** 지면 각도에 대한 자세 매칭 루프 **/
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
