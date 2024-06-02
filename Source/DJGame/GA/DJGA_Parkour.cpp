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

	//1. �ʱ�ȭ
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
	//2. ���� ��ֹ� Ȯ��
	float CapsuleHalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float Range = 150.f;
	float HalfHeight = (MaxHeight - CapsuleHalfHeight) * 0.5f;	// �ݸ����� ĸ�� �ݳ���
	FVector ForwardDir = Character->GetActorForwardVector();
	
	FHitResult ForwardCheck;
	FVector Start = Character->GetActorLocation() + FVector(0.0f, 0.0f, HalfHeight);		// �ִ� ���� ���� ĳ�� �߾ӱ��� ������ ĸ���� �������� �߻�
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
	

	//3. ��ֹ� ���� + ���� ����
	uint8 MaxIndex = 4;
	uint8 EndIndex = 0;		// ��ֹ��� �Ѿ �浹 �ε��� ��
	float HeightObstacle = 0.f;
	float Interval = 70.f;

	for (int i = 0; i <= MaxIndex; ++i)
	{
		float MaxHeightRange = 250.f;	//�ѱ� �Ұ����� ���� ����
		FHitResult HeightCheck;
		Start = FVector(ForwardCheck.ImpactPoint.X, ForwardCheck.ImpactPoint.Y,
			Character->GetActorLocation().Z - CapsuleHalfHeight + MaxHeightRange) + ForwardDir * Interval * i;		// �Ұ��� ���� ���� ĳ�� �߾� ���̱��� ��ü �߻� + interval�������� ���� �̵�
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

		if ((i == 0 && bResult == false) ||																								//ù ������ �����ϰų�
			(i == 0 && bResult && (HeightCheck.ImpactPoint.Z - (Character->GetActorLocation().Z - CapsuleHalfHeight) >= MaxHeight)))	// ���� ���̸� �����Ƽ ���
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}

		if (i == 0)	//ù��° ���� ����
		{
			VaultStartPos = HeightCheck.ImpactPoint;
			HeightObstacle = HeightCheck.ImpactPoint.Z - (Character->GetActorLocation().Z - CapsuleHalfHeight);		//�ٴں��� ��ֹ� ���볢���� ����
		}
		else
		{
			if (bResult == false)
			{
				//ù��° �浹 ����, ��ֹ��� ����� ���� ���� ��� ( 1���� �� ���濡 ����)
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
				VaultMiddlePos = HeightCheck.ImpactPoint;	//  ��ֹ� ���� ������ ��ġ ��� ����
			}
		}

	}

	// 4. ���� ����
	//������ ��Ÿ��
	UAnimMontage* SetMontage = nullptr;

	//���� ��ֹ� ����
	EHeightType currentHeightType;
	currentHeightType = HeightObstacle > MiddleHeight ? EHeightType::High : (HeightObstacle > LowHeight ? EHeightType::Middle : EHeightType::Low);


	auto CurrentMontageType = Montages.FindRef(currentHeightType);
	// EndIndex = 0�̸� vault �Ұ� (��� mantle), 1 �̸� ���� ��ֹ� , 2�̻��̸� ���� ��ֹ� //
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

	//��Ÿ�� ����
	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	ProcessMontage(SetMontage);
}

void UDJGA_Parkour::ProcessInJump(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	//�������� ����(���� ����) �϶�, ���� �����ϸ�?
	//1. �ϴ� ��ٸ�. ĳ���� �������� ������

	UAbilityTask_WaitOverlap* WaitOverlap = UAbilityTask_WaitOverlap::WaitForOverlap(this);
	WaitOverlap->OnOverlap.AddDynamic(this, &ThisClass::OverlapCallback);
	WaitOverlap->ReadyForActivation();
}

void UDJGA_Parkour::TryMantling(FHitResult& Result)
{
	//2. �𼭸� ���� Ȯ��
	float CapsuleHalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float LimitHeight = 210;	//�Ұ��� ���� ���� trace

	FHitResult HeightCheck;
	FVector Start = FVector(Result.ImpactPoint.X, Result.ImpactPoint.Y,
		Character->GetActorLocation().Z - CapsuleHalfHeight + LimitHeight);									// �Ұ��� ���� ���� �߱��� ��ü Ʈ���̽� �߻�
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

	//�𼭸��� ���ų� �ʹ� ������ ���
	if (bResult == false || (HeightCheck.ImpactPoint.Z - End.Z) > 200)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	VaultStartPos = VaultEndPos = HeightCheck.ImpactPoint;

	//3. Mantling ���� ����
	float Distance = HeightCheck.ImpactPoint.Z - End.Z;	//�߿��� �𼭸����� ���� �Ÿ� (������ ���� ���� �Ÿ�, �ִ� 0)

	UAnimMontage* SetMontage = nullptr;
	auto CurrentMontageType = Montages.FindRef(EHeightType::High);
	SetMontage = CurrentMontageType.Datas.Num() > 0 ? CurrentMontageType.Datas[0] : nullptr;

	if (SetMontage == nullptr)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	//���� �Ÿ��� ���� �ٸ��� �� ���� �ð� ���ϱ�
	float StartTime = 0.f;
	auto Curves = SetMontage->GetAnimCompositeSection(0).GetLinkedSequence()->GetCurveData().FloatCurves;
	for (FFloatCurve Curve : Curves)
	{
		if (Curve.GetName().Compare(FName(TEXT("DistanceBone"))) == 0)	//�ִϸ��̼� �� ���������� �̵��� ���� �Ÿ� Ŀ��
		{
			float MinValue, MaxValue = 0.f;
			Curve.FloatCurve.GetValueRange(MinValue, MaxValue);
			float MoveDistance = MaxValue - Distance;	// ���� �̵� �Ÿ� - ���� �Ÿ� = �̵��� �Ÿ�     //+(���� �����Ÿ��� - ���, �׳� maxĿ�갪�� �ð� ���)

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

	//��Ÿ�� ����
	if (CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo) == false)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	ProcessMontage(SetMontage, StartTime);
}

void UDJGA_Parkour::TryWallRun(FHitResult& Result, bool Left)
{
	//�켱 ���� ���������� Ȯ��
	float CheckWallNormal = Result.ImpactNormal.Dot(FVector::UpVector);
	if ((-0.0871 < CheckWallNormal && CheckWallNormal < 0.0871) == false)	//�� normal�� 85~95�� �ƴϸ� ���
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	/* �� Ÿ�� ���� */
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
	//��Ÿ�� ���� ��, �ʱ�ȭ
	Character->SetActorEnableCollision(false);
	Character->UnCrouch();
	Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	CurrentActorInfo->AbilitySystemComponent.Get()->AddLooseGameplayTag(Tag_Status_Parkour);
	
	//��ǿ��� ����
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

	//��Ÿ�� ���� �⺻ �����Ƽ �׽�ũ ���
	//Rate�� ���� ���� �̸� �߰� ����
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
	//* [���� ��, ���� ����] ��򰡿� ������ �Ǹ� �𼭸����� Ȯ�� *//

	//�ε�ģ ������ ������ ���.
	if (UAbilitySystemBlueprintLibrary::TargetDataHasHitResult(TargetData, 0) == false)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	FHitResult Result = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(TargetData, 0);
	
	FVector Dir = Result.ImpactPoint - Character->GetActorLocation();	//�ε�ģ ����
	Dir.Z = 0;
	Dir.Normalize();

	FVector Velocity = Character->GetVelocity();
	float SquaredSpeed = Velocity.SquaredLength();
	

	//�ε�ģ ������ ���� ���� �Ʒ��� �׳� ��� (����)
	if (Dir.Dot(FVector::DownVector) > 0.96 )			//�ε�ģ ������ ���� ���� �Ʒ��� �׳� ��� (����)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	/* ���� ���� ����, ���� */
	if (Character->GetActorForwardVector().Dot(Dir) >= 0.9)	//���� ���� �� 25�� ���ϸ� �ö󼭱�
		TryMantling(Result);
	// Wall Run ����....
	else if (Character->GetVelocity().SquaredLength() > 10000 && Character->GetActorForwardVector().Dot(Dir) > -0.5 &&		//�ӵ� 100 �̻�, ĳ���� ����� �ε��� ������ ������� ��ġ,
		Velocity.Dot(Character->GetActorForwardVector()) > 0 && Velocity.Z >= -100)											//�ӵ������ ĳ���� ���� ��ġ, ������ �������� ���� �ƴҶ� WallRun
	{
		bool Left = Character->GetActorForwardVector().Cross(Dir).Dot(FVector::UpVector) < 0;	// ĳ�� ����� �ε��� ���� ���� �� UpVector�� �����Ѱ� ������ ����.
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