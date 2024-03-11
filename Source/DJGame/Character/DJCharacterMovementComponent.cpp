// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/DJCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"

namespace DJCharacter
{
	//�ܼ� ������. [����� ��]
	static float GroundTraceDistance = 100000.0f;
	FAutoConsoleVariableRef CVar_GroundTraceDistance(TEXT("LyraCharacter.GroundTraceDistance"), GroundTraceDistance, 
		TEXT("Distance to trace down when generating ground information."), ECVF_Cheat);
};

UDJCharacterMovementComponent::UDJCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	:UCharacterMovementComponent(ObjectInitializer)
{
	CustomMaxJogSpeed = 550.f;
	CustomMaxWalkSpeed = 300.f;

	MaxWalkSpeed = CustomMaxJogSpeed;
}

const FLyraCharacterGroundInfo& UDJCharacterMovementComponent::GetGroundInfo()
{
	// TODO: ���⿡ return ���� �����մϴ�.
	if (!CharacterOwner || (GFrameCounter == CachedGroundInfo.LastUpdateFrame))
	{
		return CachedGroundInfo;
	}

	if (MovementMode == MOVE_Walking)
	{
		CachedGroundInfo.GroundHitResult = CurrentFloor.HitResult;
		CachedGroundInfo.GroundDistance = 0.0f;
	}
	else
	{
		const UCapsuleComponent* CapsuleComp = CharacterOwner->GetCapsuleComponent();
		check(CapsuleComp);

		const float CapsuleHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
		
		//�ݸ��� ä�� ����
		const ECollisionChannel CollisionChannel = (UpdatedComponent ? UpdatedComponent->GetCollisionObjectType() : ECC_Pawn);
		
		//��ġ ����
		const FVector TraceStart(GetActorLocation());
		const FVector TraceEnd(TraceStart.X, TraceStart.Y, (TraceStart.Z - DJCharacter::GroundTraceDistance - CapsuleHalfHeight));

		//����Param ����. (�� �������� ������ ����)
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LyraCharacterMovementComponent_GetGroundInfo), false, CharacterOwner);
		//����Param ����.
		FCollisionResponseParams ResponseParam;
		InitCollisionParams(QueryParams, ResponseParam);

		//�浹 ��� �����
		FHitResult HitResult;
		//����Ʈ���̽� ����.
		GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, CollisionChannel, QueryParams, ResponseParam);

		CachedGroundInfo.GroundHitResult = HitResult;
		CachedGroundInfo.GroundDistance = DJCharacter::GroundTraceDistance;

		if (MovementMode == MOVE_NavWalking)
		{
			CachedGroundInfo.GroundDistance = 0.0f;
		}
		else if (HitResult.bBlockingHit)
		{
			// ���� �߾ӿ��� ���������Ƿ� ĸ�� �� ���� ����.
			CachedGroundInfo.GroundDistance = FMath::Max((HitResult.Distance - CapsuleHalfHeight), 0.0f);
		}
	}

	CachedGroundInfo.LastUpdateFrame = GFrameCounter;

	return CachedGroundInfo;
}
