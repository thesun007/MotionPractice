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
	/*앉음으로 바뀐 높이를 천천히 적용위해 낮아진 위치만큼 "더할 값(ApplyCapsuleHalfHeight)"을 구하고,
	그 더하는 정도를 "0을 목표"로 보간해서 최종적용.*/

	//현재 캡슐 높이
	float TargetHalfHeight = GetOwner()->FindComponentByClass<UCapsuleComponent>()
		->GetUnscaledCapsuleHalfHeight();

	if (FMath::IsNearlyEqual(TargetHalfHeight, AppliedCapsuleHalfHeight) == false)
	{
		//더할 보간값의 초기값 구함(= 캡슐의 높이 변화값)
		if( FMath::IsNearlyZero(FinalDelta))
			ApplyCapsuleHalfHeight = AppliedCapsuleHalfHeight - TargetHalfHeight;
		else
			ApplyCapsuleHalfHeight = AppliedCapsuleHalfHeight - TargetHalfHeight + FinalDelta;	//FinalDelta가 0이 아닌것은 보간 중에 다시 높이변화가 발생한 것.
		
		AppliedCapsuleHalfHeight = TargetHalfHeight;
		blendDeltaTime = 0;
	}

	//보간 이동할 목표값 자체를 선형 보간하여 목표도달을 단계적으로 제한함으로 더욱 부드러운 효과를 냄. (exp가 1이면 선형 보간임)
	CrouchOffsetblendDeltaTime = FMath::Min(blendDeltaTime + DeltaTime * 5, 1.f);
	float CurrentCrouchOffset = FMath::InterpEaseOut(ApplyCapsuleHalfHeight, 0.f, CrouchOffsetblendDeltaTime, 1.f);

	//단계적으로 제한된 목표값을 다시 easeOut 보간하여 뒤로 갈수록 더 느리게 최종 보간 목표값에 도달하도록 함.
	blendDeltaTime = FMath::Min(blendDeltaTime + DeltaTime, 1.f);
	BlendWeightHalfHeight = FMath::InterpEaseOut(0.f, 1.f, blendDeltaTime, 3.f);
	FinalDelta = FMath::Lerp(CurrentCrouchOffset, 0.f, BlendWeightHalfHeight);
	
	//월드기준 수직 방향을 로컬 방향기준으로 변환
	FVector VerticalWorldDir = FRotationMatrix::Make(GetComponentRotation().GetInverse()).TransformVector(FVector::UpVector);
	VerticalWorldDir.Normalize();
	VerticalWorldDir *= FinalDelta;

	//더할 보간 값이 로컬기준이므로 항상 초기 로컬 위치에서 보간된 값을 더함.
	//더할 보간 값은 항상 0이 되려 하므로, 마지막엔 결국 카메라는 초기위치로 되돌아감.
	FVector SetCurrentLocalLocation = InitLocalLocation + VerticalWorldDir;
	SetRelativeLocation(FVector(SetCurrentLocalLocation.X, SetCurrentLocalLocation.Y, SetCurrentLocalLocation.Z));

	Super::GetCameraView(DeltaTime, DesiredView);
}
