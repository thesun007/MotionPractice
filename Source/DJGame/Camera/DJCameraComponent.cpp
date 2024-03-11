// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/DJCameraComponent.h"

#include "Components/CapsuleComponent.h"

UDJCameraComponent::UDJCameraComponent()
{	
	ApplyCapsuleHalfHeight = BlendWeightHalfHeight= FinalDelta = 0;
	blendDeltaTime = 1.f;
	bWantsInitializeComponent = true;
}

void UDJCameraComponent::InitializeComponent()
{
	if (GetOwner())
	{
		AppliedCapsuleHalfHeight = GetOwner()->FindComponentByClass<UCapsuleComponent>()
			->GetUnscaledCapsuleHalfHeight();
	}
	InitLocalLocation = GetRelativeLocation();
}

void UDJCameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
	float TargetHalfHeight = GetOwner()->FindComponentByClass<UCapsuleComponent>()
		->GetUnscaledCapsuleHalfHeight();

	if (FMath::IsNearlyEqual(TargetHalfHeight, AppliedCapsuleHalfHeight) == false)
	{
		//
		if( FMath::IsNearlyZero(FinalDelta))
			ApplyCapsuleHalfHeight = AppliedCapsuleHalfHeight - TargetHalfHeight;
		else
			ApplyCapsuleHalfHeight = AppliedCapsuleHalfHeight - TargetHalfHeight + FinalDelta;
		
		AppliedCapsuleHalfHeight = TargetHalfHeight;
		blendDeltaTime = 0;
	}

	/*앉음으로 바뀐 높이를 천천히 적용위해 낮아진 위치만큼 "더할 값"을 구하고,
	그 더하는 정도를 "0을 목표"로 보간해서 최종적용.*/

	//보간 이동할 목표값 자체를 선형 보간하여 목표도달을 단계적으로 제한하여 더욱 부드러운 효과를 냄.
	CrouchOffsetblendDeltaTime = FMath::Min(blendDeltaTime + DeltaTime * 5, 1.f);
	float CurrentCrouchOffset = FMath::InterpEaseOut(ApplyCapsuleHalfHeight, 0.f, CrouchOffsetblendDeltaTime, 1.f);

	//단계적으로 제한된 목표값을 easeOut 보간하여 뒤로 갈수록 더 느리게 목표값에 도달하도록 함.
	blendDeltaTime = FMath::Min(blendDeltaTime + DeltaTime, 1.f);
	BlendWeightHalfHeight = FMath::InterpEaseOut(0.f, 1.f, blendDeltaTime, 3.f);
	FinalDelta = FMath::Lerp(CurrentCrouchOffset, 0.f, BlendWeightHalfHeight);
	
	//월드기준 수직 방향을 로컬 방향기준으로 변환
	FVector VerticalWorldDir = FRotationMatrix::Make(GetComponentRotation().GetInverse()).TransformVector(FVector::UpVector);
	VerticalWorldDir.Normalize();
	VerticalWorldDir *= FinalDelta;

	//
	FVector SetCurrentLocalLocation = InitLocalLocation + VerticalWorldDir;
	SetRelativeLocation(FVector(SetCurrentLocalLocation.X, SetCurrentLocalLocation.Y, SetCurrentLocalLocation.Z));

	Super::GetCameraView(DeltaTime, DesiredView);
}
