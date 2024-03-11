// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/DJCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"

namespace DJCharacter
{
	//콘솔 변수용. [디버그 용]
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
	// TODO: 여기에 return 문을 삽입합니다.
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
		
		//콜리전 채널 변수
		const ECollisionChannel CollisionChannel = (UpdatedComponent ? UpdatedComponent->GetCollisionObjectType() : ECC_Pawn);
		
		//위치 변수
		const FVector TraceStart(GetActorLocation());
		const FVector TraceEnd(TraceStart.X, TraceStart.Y, (TraceStart.Z - DJCharacter::GroundTraceDistance - CapsuleHalfHeight));

		//쿼리Param 변수. (맨 마지막은 무시할 액터)
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LyraCharacterMovementComponent_GetGroundInfo), false, CharacterOwner);
		//응답Param 변수.
		FCollisionResponseParams ResponseParam;
		InitCollisionParams(QueryParams, ResponseParam);

		//충돌 결과 저장용
		FHitResult HitResult;
		//라인트레이스 시작.
		GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, CollisionChannel, QueryParams, ResponseParam);

		CachedGroundInfo.GroundHitResult = HitResult;
		CachedGroundInfo.GroundDistance = DJCharacter::GroundTraceDistance;

		if (MovementMode == MOVE_NavWalking)
		{
			CachedGroundInfo.GroundDistance = 0.0f;
		}
		else if (HitResult.bBlockingHit)
		{
			// 액터 중앙에서 측정했으므로 캡슐 반 길이 빼기.
			CachedGroundInfo.GroundDistance = FMath::Max((HitResult.Distance - CapsuleHalfHeight), 0.0f);
		}
	}

	CachedGroundInfo.LastUpdateFrame = GFrameCounter;

	return CachedGroundInfo;
}
