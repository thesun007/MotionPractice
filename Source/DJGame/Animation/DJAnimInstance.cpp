// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/DJAnimInstance.h"
#include "GameFramework/Character.h"
#include "Character/DJCharacterMovementComponent.h"
#include "AbilitySystemGlobals.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#if WITH_EDITOR
#include "Misc/DataValidation.h"
#include "Animation/AnimNode_StateMachine.h"
#endif

UDJAnimInstance::UDJAnimInstance()
{
	YawOffestClamp = FVector2D(-120., 100.);
	YawOffestClampInCrouch = FVector2D(-90., 80.);
	LocalDirDeadZone = 10.f;
}

void UDJAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerChar = Cast<ACharacter>(GetOwningActor());
	if (OwnerChar)
		Movement = Cast<UDJCharacterMovementComponent>( OwnerChar->GetCharacterMovement());

	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerChar))
	{
		InitializeWithAbilitySystem(ASC);
	}
}

void UDJAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	if (!OwnerChar)
		return;

	BeforeYaw = OwnerChar->GetActorRotation().Yaw;
	WorldLocation = OwnerChar->GetActorLocation();
}



void UDJAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	if (!OwnerChar)
		return;

	UpdateLocation(DeltaSeconds);
	UpdateVelocity();
	UpdateYawOffset(DeltaSeconds); 

	//땅과의 거리 계산
	const FLyraCharacterGroundInfo& GroundInfo = Movement->GetGroundInfo();
	GroundDistance = GroundInfo.GroundDistance;

	////YawOffset 디버그
	//FRotator YawOffSet = GetOwningActor()->GetActorRotation().Add(0, YawOffset, 0);
	//FVector YawOffSetDir = FRotationMatrix::Make(YawOffSet).GetUnitAxis(EAxis::X);
	//DrawDebugDirectionalArrow(GetWorld(), GetOwningActor()->GetActorLocation(),
	//	GetOwningActor()->GetActorLocation() + YawOffSetDir * 100, 10, FColor::Yellow);
}

void UDJAnimInstance::InitializeWithAbilitySystem(UAbilitySystemComponent* ASC)
{
	check(ASC);

	GameplayTagPropertyMap.Initialize(this, ASC);
}

EDataValidationResult UDJAnimInstance::IsDataValid(FDataValidationContext& Context) const
{
	Super::IsDataValid(Context);

	GameplayTagPropertyMap.IsDataValid(this, Context);

	return ((Context.GetNumErrors() > 0) ? EDataValidationResult::Invalid : EDataValidationResult::Valid);
}

void UDJAnimInstance::UpdateLocation(float DeltaSeconds)
{
	if (!OwnerChar)
		return;

	FVector NowLocation = OwnerChar->GetActorLocation();
	displacementLength = (NowLocation - WorldLocation).Size2D();
	displacementSpeed = displacementLength / DeltaSeconds;
	WorldLocation = NowLocation;
}

void UDJAnimInstance::UpdateVelocity()
{
	if (!OwnerChar)
		return;

	bool WasMovingLastUpdate = LocalVelocity2D.IsNearlyZero() ? false : true;

	WorldAccel = Movement->GetCurrentAcceleration();
	WorldVelocity = Movement->Velocity;
	GroundSpeed = WorldVelocity.Size2D();

	bHasAcceleration = WorldAccel.SizeSquared2D() > 0.f ? true : false;
	bHasVelocity = GroundSpeed > 0.f ? true : false;

	//점프 고점까지 남은 시간( 시 = 거/속)
	TimeToJumpApex = -WorldVelocity.Z / Movement->GetGravityZ();
	
	//*************************** update dir ****************************//
	WorldVelocity_2D = FVector(WorldVelocity.X, WorldVelocity.Y, 0.);
	LocalVelocity2D =  FRotationMatrix::Make(OwnerChar->GetActorRotation()).Inverse().TransformVector(WorldVelocity_2D);
	WorldAccel_2D = FVector(WorldAccel.X, WorldAccel.Y, 0.);
	LocalAccel2D = FRotationMatrix::Make(OwnerChar->GetActorRotation()).Inverse().TransformVector(WorldAccel_2D);
	
	//로컬 velocity dir
	if (!WorldVelocity_2D.IsNearlyZero())
	{
		FRotator LocalDirRot = FRotationMatrix::MakeFromX(LocalVelocity2D).Rotator();
			//(FRotationMatrix::MakeFromX(WorldVelocity_2D).Rotator() - OwnerChar->GetActorRotation());
		LocalDirRot.Normalize();
		LocalDirAngle = LocalDirRot.Yaw;
	}
	else
	{
		LocalDirAngle = 0.f;
	}

	LocalDirAngleWithYawOffset = LocalDirAngle - YawOffset;
	 
	LocalDirWithYawOffset = UpdateDir(LocalDirAngleWithYawOffset, LocalDirDeadZone, 
		LocalDirWithYawOffset, WasMovingLastUpdate);
	//UE_LOG(LogTemp, Log, TEXT("LocalDirWithYawOffset : %s"), *UEnum::GetValueAsString(LocalDirWithYawOffset) );
	
	//로컬 acceleration dir
	if (!WorldAccel_2D.IsNearlyZero())
	{
		FRotator LocalAccelDirRot = (FRotationMatrix::MakeFromX(WorldAccel_2D).Rotator() - OwnerChar->GetActorRotation());
		LocalAccelDirRot.Normalize();
		LocalAccelDirAngle = LocalAccelDirRot.Yaw;
	}
	else
	{
		LocalAccelDirAngle = 0.f;
	}

	LocalAccelDirAngleWithYawOffset = LocalAccelDirAngle - YawOffset;

	LocalAccelDirWithYawOffset = UpdateDir(LocalAccelDirAngleWithYawOffset, LocalDirDeadZone,
		LocalAccelDirWithYawOffset, WasMovingLastUpdate);

	ReverseDir(LocalAccelDirWithYawOffset);	//Pivot에 사용

	//캐릭터 상태 업데이트
	UpdateState(WasMovingLastUpdate);
}

EMoveDir UDJAnimInstance::UpdateDir(float localAngle, float DirDeadZone, EMoveDir BeforeLocalDir, bool WasMovingLastUpdate)
{
	float FwdDeadZone = DirDeadZone;
	float BwdDeadZone = DirDeadZone;
	float AbsLocalAngle = FMath::Abs(localAngle);

	if (WasMovingLastUpdate)
	{
		if (BeforeLocalDir == EMoveDir::Forward)
			FwdDeadZone *= 2.f;
		else if(BeforeLocalDir == EMoveDir::Backward)
			BwdDeadZone *= 2.f;
	}

	if (AbsLocalAngle <= (45.0 + FwdDeadZone))
		return EMoveDir::Forward;
	else if (AbsLocalAngle >= (135 - BwdDeadZone))
		return EMoveDir::Backward;
	else if (localAngle > 0)
		return EMoveDir::Right;
	else
		return EMoveDir::Left;
}

void UDJAnimInstance::UpdateState(bool WasMovingLastUpdate)
{
	if (!OwnerChar)
		return;

	bIsFall = Movement->IsFalling();
	bIsJump = bIsFall & (WorldVelocity.Z > 0);

	uint8 curCrouch = Movement->IsCrouching();
	bCrouchChange = curCrouch != bIsCrouch ? true : false;
	bIsCrouch = curCrouch;
	//UE_LOG(LogTemp, Log, TEXT("bHasVelocity : %d || bHasAcceleration : %d"), bHasVelocity, bHasAcceleration);
	//UE_LOG(LogTemp, Log, TEXT("Velocity : [%f] || Acceleration : [%f]"), LocalVelocity2D.Y, LocalAccel2D.Y);
	if (bHasVelocity && bHasAcceleration)
	{
		MoveState = EMoveState::move;

		if (WasMovingLastUpdate && (LocalVelocity2D.Dot(LocalAccel2D) < 0.))
		{
			MoveState = EMoveState::pivot;
		}
	}
	else if (WasMovingLastUpdate && bHasVelocity && !bHasAcceleration)
	{
		MoveState = EMoveState::stop;
	}
	else
	{
		MoveState = EMoveState::idle;
	}

	//UE_LOG(LogTemp, Log, TEXT("MoveState : %s"), *UEnum::GetValueAsString(MoveState))
}

void UDJAnimInstance::ReverseDir(EMoveDir& TargetDir)
{
	if (TargetDir == EMoveDir::Forward) TargetDir = EMoveDir::Backward;
	else if(TargetDir == EMoveDir::Backward) TargetDir = EMoveDir::Forward;
	else if (TargetDir == EMoveDir::Left) TargetDir = EMoveDir::Right;
	else if (TargetDir == EMoveDir::Right) TargetDir = EMoveDir::Left;
}

void UDJAnimInstance::UpdateYawOffset(float DeltaSeconds)
{
	if (!OwnerChar)
		return;
	
	float currentYaw = OwnerChar->GetActorRotation().Yaw;
	YawDelta = currentYaw - BeforeYaw;
	BeforeYaw = currentYaw;
	YawSpeed = YawDelta / DeltaSeconds * 0.035;	// 0.035 임시
	//UE_LOG(LogTemp, Log, TEXT("YawSpeed : %f"), YawSpeed);
	if (GetCurrentActiveMontage() != nullptr)		//몽타주 실행중이 아니면 BlendOut
	{
		YawOffsetMode = EYawOffsetMode::BlendOut;

	}
	if (YawOffsetMode == EYawOffsetMode::Execute)
	{
		YawOffset -= YawDelta;
		SetYawOffset(YawOffset);
	}
	else if (YawOffsetMode == EYawOffsetMode::BlendOut)
	{
		//Stiffness
		float springvalue = UKismetMathLibrary::FloatSpringInterp(YawOffset, 0.f, YawOffsetSpringState, 80.0f, 1.0f,
			DeltaSeconds, 1.0f, 0.5f);
		SetYawOffset(springvalue);
		//UE_LOG(LogTemp, Log, TEXT("YawOffset : %f"), springvalue);
	}

	YawOffsetMode = EYawOffsetMode::BlendOut;
}

void UDJAnimInstance::SetYawOffset(float inYawOffset)
{
	if(bIsCrouch)
		YawOffset = FMath::ClampAngle(FRotator::NormalizeAxis(inYawOffset), YawOffestClampInCrouch.X, YawOffestClampInCrouch.Y);
	else
		YawOffset = FMath::ClampAngle(FRotator::NormalizeAxis(inYawOffset), YawOffestClamp.X, YawOffestClamp.Y);
	
	AimYaw = -YawOffset;
	AimPitch = FRotator::NormalizeAxis(OwnerChar->GetControlRotation().Pitch); /*OwnerChar->GetBaseAimRotation().Pitch*/
}

void UDJAnimInstance::ProcessTurnCurve()
{
	float TurnYawWeight = 0.f;
	float PreviousTurnCurveValue = CurrentTurnCurveValue;

	bool IsCurrentTurning = GetCurveValue(TEXT("TurnYawWeight"), TurnYawWeight);

	if (IsCurrentTurning == false || TurnYawWeight <0.8)
	{
		CurrentTurnCurveValue = 0.f;
		return;
	}

	float RemainingTurnYaw = 0.f;
	GetCurveValue(TEXT("RemainingTurnYaw"), RemainingTurnYaw);

	CurrentTurnCurveValue = RemainingTurnYaw / TurnYawWeight;

	if (PreviousTurnCurveValue != 0.f)
	{
		float TurnDelta = PreviousTurnCurveValue -CurrentTurnCurveValue;

		SetYawOffset(YawOffset + TurnDelta);

		//UE_LOG(LogTemp, Log, TEXT("TurnDelta : %f  [Pre : %f][Current : %f]"), TurnDelta, PreviousTurnCurveValue, CurrentTurnCurveValue);
	}
}

UDJAnimInstance* UDJAnimInstance::GetMainAnimIns() const
{
	return const_cast<UDJAnimInstance*>(this);
	
}
	
