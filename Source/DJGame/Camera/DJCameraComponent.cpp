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
	/*�������� �ٲ� ���̸� õõ�� �������� ������ ��ġ��ŭ "���� ��(ApplyCapsuleHalfHeight)"�� ���ϰ�,
	�� ���ϴ� ������ "0�� ��ǥ"�� �����ؼ� ��������.*/

	//���� ĸ�� ����
	float TargetHalfHeight = GetOwner()->FindComponentByClass<UCapsuleComponent>()
		->GetUnscaledCapsuleHalfHeight();

	if (FMath::IsNearlyEqual(TargetHalfHeight, AppliedCapsuleHalfHeight) == false)
	{
		//���� �������� �ʱⰪ ����(= ĸ���� ���� ��ȭ��)
		if( FMath::IsNearlyZero(FinalDelta))
			ApplyCapsuleHalfHeight = AppliedCapsuleHalfHeight - TargetHalfHeight;
		else
			ApplyCapsuleHalfHeight = AppliedCapsuleHalfHeight - TargetHalfHeight + FinalDelta;	//FinalDelta�� 0�� �ƴѰ��� ���� �߿� �ٽ� ���̺�ȭ�� �߻��� ��.
		
		AppliedCapsuleHalfHeight = TargetHalfHeight;
		blendDeltaTime = 0;
	}

	//���� �̵��� ��ǥ�� ��ü�� ���� �����Ͽ� ��ǥ������ �ܰ������� ���������� ���� �ε巯�� ȿ���� ��. (exp�� 1�̸� ���� ������)
	CrouchOffsetblendDeltaTime = FMath::Min(blendDeltaTime + DeltaTime * 5, 1.f);
	float CurrentCrouchOffset = FMath::InterpEaseOut(ApplyCapsuleHalfHeight, 0.f, CrouchOffsetblendDeltaTime, 1.f);

	//�ܰ������� ���ѵ� ��ǥ���� �ٽ� easeOut �����Ͽ� �ڷ� ������ �� ������ ���� ���� ��ǥ���� �����ϵ��� ��.
	blendDeltaTime = FMath::Min(blendDeltaTime + DeltaTime, 1.f);
	BlendWeightHalfHeight = FMath::InterpEaseOut(0.f, 1.f, blendDeltaTime, 3.f);
	FinalDelta = FMath::Lerp(CurrentCrouchOffset, 0.f, BlendWeightHalfHeight);
	
	//������� ���� ������ ���� ����������� ��ȯ
	FVector VerticalWorldDir = FRotationMatrix::Make(GetComponentRotation().GetInverse()).TransformVector(FVector::UpVector);
	VerticalWorldDir.Normalize();
	VerticalWorldDir *= FinalDelta;

	//���� ���� ���� ���ñ����̹Ƿ� �׻� �ʱ� ���� ��ġ���� ������ ���� ����.
	//���� ���� ���� �׻� 0�� �Ƿ� �ϹǷ�, �������� �ᱹ ī�޶�� �ʱ���ġ�� �ǵ��ư�.
	FVector SetCurrentLocalLocation = InitLocalLocation + VerticalWorldDir;
	SetRelativeLocation(FVector(SetCurrentLocalLocation.X, SetCurrentLocalLocation.Y, SetCurrentLocalLocation.Z));

	Super::GetCameraView(DeltaTime, DesiredView);
}
