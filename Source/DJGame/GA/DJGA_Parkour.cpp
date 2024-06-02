// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/DJGA_Parkour.h"
#include "DrawDebugHelpers.h"
#include "Character/DJCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "GAS/DJAbilitySystemComponent.h"
#include "Abilities\Tasks\AbilityTask_PlayMontageAndWait.h"
#include "MotionWarpingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitOverlap.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AT/DJAT_WallRun.h"
#include "Tag/DJGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_InputTag_Parkour, "InputTag.Ability.Parkour");
UE_DEFINE_GAMEPLAY_TAG(TAG_AbilityType_Action_Parkour, "Ability.Type.Action.Parkour");

UDJGA_Parkour::UDJGA_Parkour(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer), VaultStartPos(), VaultMiddlePos(), VaultEndPos()
{
	AbilityTags.AddTag(TAG_AbilityType_Action_Parkour);
	FAbilityTriggerData trigger;
	trigger.TriggerTag = TAG_InputTag_Parkour;
	AbilityTriggers.Add(trigger);
}

void UDJGA_Parkour::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	//1. 초기화
	Character = GetDJCharacterFromActorInfo();
	VaultStartPos = VaultMiddlePos = VaultEndPos = FVector::ZeroVector;
	VaultStartRot = FRotator::ZeroRotator;

	if (Character == nullptr)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	if (Character->GetCharacterMovement()->IsFalling())
		ProcessInJump(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	else
		ProcessInNormal(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UDJGA_Parkour::ProcessInNormal(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	//2. 전방 장애물 확인
	float CapsuleHalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float Range = 150.f;
	float HalfHeight = (MaxHeight - CapsuleHalfHeight) * 0.5f;	// 콜리전용 캡슐 반높이
	FVector ForwardDir = Character->GetActorForwardVector();
	
	FHitResult ForwardCheck;
	FVector Start = Character->GetActorLocation() + FVector(0.0f, 0.0f, HalfHeight);		// 최대 높이 부터 캐릭 중앙까지 길이의 캡슐을 전방으로 발사
	FVector End = Start + ForwardDir * Range;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(FirstCheck_Parkour), false, Character);

	bool bResult = GetWorld()->SweepSingleByChannel(ForwardCheck, Start, End, FQuat::Identity, ECollisionChannel::ECC_Visibility,
		FCollisionShape::MakeCapsule(10.f, HalfHeight), Params);

#if ENABLE_DRAW_DEBUG
	if (bDebug)
	{
		DrawDebugCapsule(GetWorld(), End, HalfHeight, 10.f, FQuat::Identity, FColor::Red,
			false, 3.0f);
		if (bResult)
			DrawDebugSphere(GetWorld(), ForwardCheck.ImpactPoint, 10.f, 16, FColor::Green,
				false, 3.0f);
	}
#endif

	if (bResult == false)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}
	

	//3. 장애물 높이 + 착지 지점
	uint8 MaxIndex = 4;
	uint8 EndIndex = 0;		// 장애물을 넘어간 충돌 인덱스 값
	float HeightObstacle = 0.f;
	float Interval = 70.f;

	for (int i = 0; i <= MaxIndex; ++i)
	{
		float MaxHeightRange = 250.f;	//넘기 불가능한 높이 설정
		FHitResult HeightCheck;
		Start = FVector(ForwardCheck.ImpactPoint.X, ForwardCheck.ImpactPoint.Y,
			Character->GetActorLocation().Z - CapsuleHalfHeight + MaxHeightRange) + ForwardDir * Interval * i;		// 불가능 높이 부터 캐릭 중앙 높이까지 구체 발사 + interval간격으로 전방 이동
		End = Start + (FVector::DownVector * MaxHeightRange) + FVector(0.0f, 0.0f, CapsuleHalfHeight);
		FCollisionQueryParams HeightParams(SCENE_QUERY_STAT(HeightCheck_Parkour), false, Character);

		bResult = GetWorld()->SweepSingleByChannel(HeightCheck, Start, End, FQuat::Identity, ECollisionChannel::ECC_Visibility,
			FCollisionShape::MakeSphere(10.f), HeightParams);

#if ENABLE_DRAW_DEBUG
		if (bDebug)
		{
			FVector CapsuleOrigin = Start + (End - Start) * 0.5f;
			DrawDebugCapsule(GetWorld(), CapsuleOrigin, (MaxHeightRange - CapsuleHalfHeight) * 0.5f, 10.f, FQuat::Identity, FColor::White,
				false, 3.0f);
			if (bResult)
				DrawDebugSphere(GetWorld(), HeightCheck.ImpactPoint, 10.f, 16, FColor::Yellow,
					false, 3.0f);
		}
#endif

		if ((i == 0 && bResult == false) ||																								//첫 지점이 실패하거나
			(i == 0 && bResult && (HeightCheck.ImpactPoint.Z - (Character->GetActorLocation().Z - CapsuleHalfHeight) >= MaxHeight)))	// 높은 벽이면 어빌리티 취소
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}

		if (i == 0)	//첫번째 지점 저장
		{
			VaultStartPos = HeightCheck.ImpactPoint;
			HeightObstacle = HeightCheck.ImpactPoint.Z - (Character->GetActorLocation().Z - CapsuleHalfHeight);		//바닥부터 장애물 꼭대끼까지 높이
		}
		else
		{
			if (bResult == false)
			{
				//첫번째 충돌 이후, 장애물을 벗어나면 착지 지점 계산 ( 1미터 더 전방에 착지)
				Start += ForwardDir * 100;
				End += FVector(0.0, 0.0, -1000) + ForwardDir * 100;
				FCollisionQueryParams LandParams(SCENE_QUERY_STAT(LandCheck_Parkour), false, Character);
				bResult = GetWorld()->LineTraceSingleByChannel(HeightCheck, Start, End, ECollisionChannel::ECC_Visibility,
					LandParams);

#if ENABLE_DRAW_DEBUG
				if (bDebug)
				{
					FVector CapsuleOrigin = Start + (End - Start) * 0.5f;
					DrawDebugLine(GetWorld(), Start, End, FColor::Blue,
						false, 3.0f);
					if (bResult)
						DrawDebugSphere(GetWorld(), HeightCheck.ImpactPoint, 5.f, 8, FColor::Purple,
							false, 3.0f);
				}
#endif
				if (bResult)
				{
					VaultEndPos = HeightCheck.ImpactPoint;

					EndIndex = i;

					break;
				}
			}
			else
			{
				VaultMiddlePos = HeightCheck.ImpactPoint;	//  장애물 면적 마지막 위치 계속 갱신
			}
		}

	}

	// 4. 파쿠르 수행
	//선택할 몽타주
	UAnimMontage* SetMontage = nullptr;

	//현재 장애물 높이
	EHeightType currentHeightType;
	currentHeightType = HeightObstacle > MiddleHeight ? EHeightType::High : (HeightObstacle > LowHeight ? EHeightType::Middle : EHeightType::Low);


	auto CurrentMontageType = Montages.FindRef(currentHeightType);
	// EndIndex = 0이면 vault 불가 (대신 mantle), 1 이면 얕은 장애물 , 2이상이면 깊은 장애물 //
	EndIndex = FMath::Min(EndIndex, (uint8)2);

	for (int i = EndIndex; i >= 0; --i)
	{
		if (CurrentMontageType.Datas.IsValidIndex(i))
		{
			SetMontage = CurrentMontageType.Datas[i];

			if (i == 0)
			{
				VaultStartRot = FRotationMatrix::MakeFromX(-ForwardCheck.ImpactNormal).Rotator();
			}
			else
			{
				VaultStartRot = Character->GetActorRotation();
			}
			break;
		}
	}

	if (SetMontage == nullptr)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	//몽타주 실행
	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	ProcessMontage(SetMontage);
}

void UDJGA_Parkour::ProcessInJump(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	//떨어지는 상태(점프 상태) 일때, 파쿠르 실행하면?
	//1. 일단 기다림. 캐릭터 오버랩될 때까지

	UAbilityTask_WaitOverlap* WaitOverlap = UAbilityTask_WaitOverlap::WaitForOverlap(this);
	WaitOverlap->OnOverlap.AddDynamic(this, &ThisClass::OverlapCallback);
	WaitOverlap->ReadyForActivation();
}

void UDJGA_Parkour::TryMantling(FHitResult& Result)
{
	//2. 모서리 높이 확인
	float CapsuleHalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float LimitHeight = 210;	//불가능 높이 부터 trace

	FHitResult HeightCheck;
	FVector Start = FVector(Result.ImpactPoint.X, Result.ImpactPoint.Y,
		Character->GetActorLocation().Z - CapsuleHalfHeight + LimitHeight);									// 불가능 높이 부터 발까지 구체 트레이스 발사
	FVector End = Start + (FVector::DownVector * LimitHeight);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(FirstCheck_Parkour), false, Character);

	bool bResult = GetWorld()->SweepSingleByChannel(HeightCheck, Start, End, FQuat::Identity, ECollisionChannel::ECC_Visibility,
		FCollisionShape::MakeSphere(10.f), Params);

#if ENABLE_DRAW_DEBUG
	if (bDebug)
	{
		FVector CapsuleOrigin = Start + (End - Start) * 0.5f;
		DrawDebugCapsule(GetWorld(), CapsuleOrigin, LimitHeight * 0.5f, 10.f, FQuat::Identity, FColor::Red,
			false, 3.0f);
		if (bResult)
			DrawDebugSphere(GetWorld(), HeightCheck.ImpactPoint, 10.f, 16, FColor::Green,
				false, 3.0f);
	}
#endif

	//모서리가 없거나 너무 높으면 취소
	if (bResult == false || (HeightCheck.ImpactPoint.Z - End.Z) > 200)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	VaultStartPos = VaultEndPos = HeightCheck.ImpactPoint;

	//3. Mantling 파쿠르 수행
	float Distance = HeightCheck.ImpactPoint.Z - End.Z;	//발에서 모서리까지 수직 거리 (목적지 까지 남은 거리, 최대 0)

	UAnimMontage* SetMontage = nullptr;
	auto CurrentMontageType = Montages.FindRef(EHeightType::High);
	SetMontage = CurrentMontageType.Datas.Num() > 0 ? CurrentMontageType.Datas[0] : nullptr;

	if (SetMontage == nullptr)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	//남은 거리에 따라 다르게 할 시작 시간 구하기
	float StartTime = 0.f;
	auto Curves = SetMontage->GetAnimCompositeSection(0).GetLinkedSequence()->GetCurveData().FloatCurves;
	for (FFloatCurve Curve : Curves)
	{
		if (Curve.GetName().Compare(FName(TEXT("DistanceBone"))) == 0)	//애니메이션 상 목적지까지 이동한 수직 거리 커브
		{
			float MinValue, MaxValue = 0.f;
			Curve.FloatCurve.GetValueRange(MinValue, MaxValue);
			float MoveDistance = MaxValue - Distance;	// 최종 이동 거리 - 남은 거리 = 이동한 거리     //+(만약 남은거리가 - 라면, 그냥 max커브값의 시간 얻기)

			for (FRichCurveKey CurvePair : Curve.FloatCurve.GetConstRefOfKeys())
			{
				if (CurvePair.Value >= MoveDistance)
				{
					StartTime = CurvePair.Time;
					break;
				}
				if (FMath::IsNearlyEqual(CurvePair.Value, MaxValue))
				{
					StartTime = CurvePair.Time;
					break;
				}
			}
			//UE_LOG(LogTemp, Log, TEXT("Distance : %f || StartTime : %f"), MoveDistance, StartTime);
			break;
		}
	}

	FVector RotationDir = VaultStartPos - Character->GetActorLocation();
	RotationDir.Z = 0;
	VaultStartRot = FRotationMatrix::MakeFromX(RotationDir).Rotator();

	//몽타주 실행
	if (CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo) == false)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	ProcessMontage(SetMontage, StartTime);
}

void UDJGA_Parkour::TryWallRun(FHitResult& Result, bool Left)
{
	//우선 벽이 수직벽인지 확인
	float CheckWallNormal = Result.ImpactNormal.Dot(FVector::UpVector);
	if ((-0.0871 < CheckWallNormal && CheckWallNormal < 0.0871) == false)	//벽 normal이 85~95가 아니면 취소
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	/* 벽 타기 시작 */
	UDJAT_WallRun* WallRun = UDJAT_WallRun::CreateWallRunTask(this, FName("WallRunTask"), Result.ImpactPoint, Result.ImpactNormal, Left, 
		Character->GetCapsuleComponent()->GetScaledCapsuleRadius()+5);

	if (WallRun == nullptr)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	if (CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo) == false)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	WallRun->Completed.AddDynamic(this, &ThisClass::EndFunction);
	WallRun->Canceled.AddDynamic(this, &ThisClass::EndFunction);
	WallRun->bDebug = bDebug;
	WallRun->ReadyForActivation();
}

void UDJGA_Parkour::ProcessMontage(UAnimMontage* _Montage, float StartTime)
{
	//몽타주 실행 전, 초기화
	Character->SetActorEnableCollision(false);
	Character->UnCrouch();
	Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	CurrentActorInfo->AbilitySystemComponent.Get()->AddLooseGameplayTag(Tag_Status_Parkour);
	
	//모션와핑 적용
	UMotionWarpingComponent* MWC = Character->GetMotionWarpingComponent();
	if (MWC)
	{
		if (VaultStartPos.IsNearlyZero() == false)
			MWC->AddOrUpdateWarpTargetFromLocationAndRotation(FName(TEXT("VaultStart")), VaultStartPos, VaultStartRot);
		if (VaultMiddlePos.IsNearlyZero() == false)
			MWC->AddOrUpdateWarpTargetFromLocationAndRotation(FName(TEXT("VaultMiddle")), VaultMiddlePos, VaultStartRot);
		if (VaultEndPos.IsNearlyZero() == false)
			MWC->AddOrUpdateWarpTargetFromLocationAndRotation(FName(TEXT("VaultLast")), VaultEndPos, VaultStartRot);
	}

	//몽타주 실행 기본 어빌리티 테스크 사용
	//Rate와 시작 섹션 이름 추가 지정
	UAbilityTask_PlayMontageAndWait* AT_DashMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("Parkour"),
		_Montage, 1.f, NAME_None, true, 1.0f, StartTime);

	Character->GetMesh()->GetAnimInstance()->OnPlayMontageNotifyEnd.AddDynamic(this, &ThisClass::MontageEndNotifyCallback);
	AT_DashMontage->OnInterrupted.AddDynamic(this, &UDJGA_Parkour::EndFunction);
	AT_DashMontage->OnCancelled.AddDynamic(this, &UDJGA_Parkour::EndFunction);
	AT_DashMontage->OnCompleted.AddDynamic(this, &UDJGA_Parkour::EndFunction);
	AT_DashMontage->ReadyForActivation();
}

void UDJGA_Parkour::MontageEndNotifyCallback(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if (NotifyName == FName(TEXT("EnableFalling")))
	{
		Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
		Character->SetActorEnableCollision(true);
	}
}

void UDJGA_Parkour::OverlapCallback(const FGameplayAbilityTargetDataHandle& TargetData)
{
	//* [점프 중, 파쿠르 결정] 어딘가에 오버랩 되면 모서리인지 확인 *//

	//부딛친 데이터 없으면 취소.
	if (UAbilitySystemBlueprintLibrary::TargetDataHasHitResult(TargetData, 0) == false)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	FHitResult Result = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(TargetData, 0);
	
	FVector Dir = Result.ImpactPoint - Character->GetActorLocation();	//부딪친 방향
	Dir.Z = 0;
	Dir.Normalize();

	FVector Velocity = Character->GetVelocity();
	float SquaredSpeed = Velocity.SquaredLength();
	

	//부딪친 방향이 거의 수직 아래면 그냥 취소 (랜딩)
	if (Dir.Dot(FVector::DownVector) > 0.96 )			//부딪친 방향이 거의 수직 아래면 그냥 취소 (랜딩)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	/* 파쿠르 종류 결정, 실행 */
	if (Character->GetActorForwardVector().Dot(Dir) >= 0.9)	//정면 기준 약 25도 이하면 올라서기
		TryMantling(Result);
	// Wall Run 조건....
	else if (Character->GetVelocity().SquaredLength() > 10000 && Character->GetActorForwardVector().Dot(Dir) > -0.5 &&		//속도 100 이상, 캐릭터 방향과 부딪힌 방향이 어느정도 일치,
		Velocity.Dot(Character->GetActorForwardVector()) > 0 && Velocity.Z >= -100)											//속도방향과 캐릭터 방향 일치, 빠르게 떨어지는 중이 아닐때 WallRun
	{
		bool Left = Character->GetActorForwardVector().Cross(Dir).Dot(FVector::UpVector) < 0;	// 캐릭 정면과 부딛힌 방향 외적 후 UpVector와 내적한게 음수면 왼쪽.
		TryWallRun(Result, Left);
	}
	else
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	
	//UE_LOG(LogTemp, Log, TEXT("Velocity : %f"), Character->GetVelocity().SquaredLength());

}

void UDJGA_Parkour::EndFunction()
{
	Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	Character->SetActorEnableCollision(true);
	Character->GetMesh()->GetAnimInstance()->OnPlayMontageNotifyEnd.RemoveDynamic(this, &ThisClass::MontageEndNotifyCallback);
	CurrentActorInfo->AbilitySystemComponent.Get()->RemoveLooseGameplayTag(Tag_Status_Parkour);

	UMotionWarpingComponent* MWC = Character->GetMotionWarpingComponent();
	if (MWC)
	{
		MWC->RemoveWarpTarget("VaultStart");
		MWC->RemoveWarpTarget("VaultMiddle");
		MWC->RemoveWarpTarget("VaultLast");
	}
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}