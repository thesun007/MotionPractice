// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/AT/DJAT_WallRun.h"
#include "DrawDebugHelpers.h"
#include "Tag/DJGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "GA/DJGameplayAbility.h"
#include "Character/DJCharacterBase.h"
#include "Character/DJCharacterMovementComponent.h"
#include "Animation/DJAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Interface/AnimSourceSetInterface.h"
#include "Kismet/KismetSystemLibrary.h"

UDJAT_WallRun::UDJAT_WallRun()
{
    bTickingTask = true;

}

void UDJAT_WallRun::TickTask(float DeltaTime)
{
    if (RunState == EWallRunState::Loop)
    {
        int32 LeftRight = bLeft ? -1 : 1;

        //장애물 막힘 확인 로직
        FVector ActorPos2D = Character->GetActorLocation(); 
        ActorPos2D.Z = 0;
        if ((BeforePosition2D - ActorPos2D).SquaredLength() < 0.5f)    //이동을 거의 못했으면..
        {
            UE_LOG(LogTemp, Log, TEXT("on Blocking"));
            OnCanceled();
            return;
        }
        BeforePosition2D = ActorPos2D;
        
        /*다음 위치 계산*/
        //예상 높이 계산 (약 포물선 이동)
        CurrentVirSpeed += (Character->GetCharacterMovement()->GetGravityZ()*0.4f ) * DeltaTime;    //중력(1/2.5) 적용
        float DeltaZ = CurrentVirSpeed * DeltaTime; //더할 Z 값
        
        //떨어지는 속도가 빨라지면 종료
        float FallAngle = FMath::RadiansToDegrees(FMath::Atan2(CurrentVirSpeed, CurrentSpeed));
        if (FallAngle < -45)  //떨어지는 각도가 -45이하
        {
            OnCanceled();
            return;
        }

        //예상 전방 위치 계산
        FVector StartPos = Character->GetActorLocation() + Character->GetActorForwardVector() * CurrentSpeed * DeltaTime;   //캐릭터 예상 전방 위치
        StartPos.Z += DeltaZ;
        FVector EndPos = StartPos + Character->GetActorRightVector()* LeftRight * (GapOffsetWithWall + 5);   // 예상 전방위치에서 벽 쪽으로 LineTrace

        FHitResult HitResult;
        FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CheckWall), false, Character);
        bool Hit = GetWorld()->LineTraceSingleByObjectType(HitResult, StartPos, EndPos, FCollisionObjectQueryParams::AllStaticObjects, QueryParams);

#if ENABLE_DRAW_DEBUG
        if (bDebug)
        {
            FVector CapsuleOrigin = StartPos + (EndPos - StartPos) * 0.5f;
            DrawDebugLine(GetWorld(), StartPos, EndPos, FColor::Yellow,
                false, 1.0f);
            if (Hit)
                DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 5.f, 8, FColor::Green,
                    false, 1.0f);
            //DrawDebugLine(GetWorld(), HitResult.ImpactPoint, HitResult.ImpactPoint+ HitResult.ImpactNormal*30, FColor::White, false, 1.0f);
        }
#endif

        if (Hit == false)   //벽이 없으면 종료
        {
            OnCanceled();
            return;
        }

        CurrentWallNormal = HitResult.ImpactNormal;
        FVector FinalPos = HitResult.ImpactPoint + HitResult.ImpactNormal * GapOffsetWithWall;  //최종 위치
        FVector ForwardWithWall = FVector::UpVector.Cross(CurrentWallNormal) * LeftRight;
        FRotator FinalRot = FRotationMatrix::MakeFromX(ForwardWithWall).Rotator();
        
        FRotator CheckRotDir = FinalRot - Character->GetActorRotation(); CheckRotDir.Normalize();   //회전해야할 방향 확인 (벽이 바깥으로 휘어져있으면 취소)
        //UE_LOG(LogTemp, Log, TEXT("CheckRot : %f"), CheckRotDir.Yaw * LeftRight);
        if (CheckRotDir.Yaw * LeftRight > 1.5f)
        {
            OnCanceled();
            return;
        }

        //캐릭터 최종 트랜스폼 적용
        Character->SetActorLocationAndRotation(FinalPos, FinalRot);

        //벽 따라 이동한 수평속도 애니메이션에 전달
        FVector Displacement2D = HitResult.ImpactPoint - CurrentWallPosition;   // (예상으론 곡선벽이라면 CurrentSpeed와 달라질 것)
        Displacement2D.Z = 0;
        AnimSourceSetInterface->SetDisplacementSpeedWithWall(Displacement2D.Length() / DeltaTime); 
        CurrentWallPosition = HitResult.ImpactPoint;    //벽 포인트 위치 갱신
        //UE_LOG(LogTemp, Log, TEXT("2D Speed : %f"), Displacement2D.Length() / DeltaTime);

        /* 착지 가능 위치 체크 */
        FHitResult GroundHitResult;
        FCollisionQueryParams GroundQueryParams(SCENE_QUERY_STAT(CheckGround), false, Character);
        FVector Start = Character->GetActorLocation();
        FVector CheckDownDir = FVector(1, 0, -1);  //왼쪽 벽이었으면 앞 오른쪽 아래 방향 
        CheckDownDir.Normalize();
        CheckDownDir = FRotationMatrix(FRotator(0, Character->GetActorRotation().Yaw + 45 * -LeftRight, 0)).TransformVector(CheckDownDir);
        
        FVector End = Start + CheckDownDir *300;    //(목표는 수직아래 거리 60이하되는 타이밍이라서 길이 100정도 잡음)

        Hit = GetWorld()->LineTraceSingleByObjectType(GroundHitResult, Start, End, FCollisionObjectQueryParams::AllStaticObjects, GroundQueryParams);   //지면 체크

#if ENABLE_DRAW_DEBUG
        if (bDebug)
        {
            FVector CapsuleOrigin = Start + (End - Start) * 0.5f;
            DrawDebugLine(GetWorld(), Start, End, FColor::Blue,
                false, 0.0f);
            if (Hit)
                DrawDebugSphere(GetWorld(), GroundHitResult.ImpactPoint, 5.f, 8, FColor::Purple,
                    false, 1.0f);
        }
#endif

        // 발 바닥 부터 지면까지 수직아래 차이가 60이하면 착지 시도
        if (FallAngle < -20 && Hit && GroundHitResult.ImpactNormal.Z > 0 &&
            (Start.Z - Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - GroundHitResult.ImpactPoint.Z) < 60) 
        {
            LandingPoint = GroundHitResult.ImpactPoint;
            RunState = EWallRunState::End;
            AbilitySystemComponent.Get()->RemoveLooseGameplayTag(Tag_Status_Parkour_WallRun);
        }
        else
        {
            AnimSourceSetInterface->SetGroundDistanceByWallRun(100);    //착지 지면 없으면 임의로 100 전송
        }
    }
    else
    {
        int32 LeftRight = bLeft ? 1 : -1;
        
        /* wallrun 착지 모션 중 움직임 진행(랜딩) */
        float GroundDistance = Character->GetActorLocation().Z - Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - LandingPoint.Z;   //발바닥부터 착지 지점까지 수직거리 전달

        if (Character->GetMesh()->GetAnimInstance()->WasAnimNotifyNameTriggeredInAnyState(FName("EnableFalling")))
        {
            OnCompleted();
            UE_LOG(LogTemp, Log, TEXT("on Falling"));
            return;
        }

        AnimSourceSetInterface->SetGroundDistanceByWallRun(GroundDistance);  
        Character->GetCharacterMovement()->Velocity = Character->GetActorForwardVector() * CurrentSpeed;
        Character->AddActorWorldOffset(FVector(0.0f, 0.0f, Character->GetCharacterMovement()->GetGravityZ())* DeltaTime*0.33f);
        Character->AddMovementInput(Character->GetActorRightVector() * LeftRight, 2.0);
    }
}

UDJAT_WallRun* UDJAT_WallRun::CreateWallRunTask(UGameplayAbility* OwningAbility, FName TaskInstanceName, FVector StartWallPoint, FVector StartWallNormal,
    bool Left, float _GapOffsetWithWall)
{
    UDJAT_WallRun* WallRunTaskInstance = NewAbilityTask<UDJAT_WallRun>(OwningAbility, TaskInstanceName);		//Register for task list here, providing a given FName as a key
    WallRunTaskInstance->bLeft = Left;
    WallRunTaskInstance->GapOffsetWithWall = _GapOffsetWithWall;
    WallRunTaskInstance->RunState = EWallRunState::Loop;
    WallRunTaskInstance->CurrentWallPosition = StartWallPoint;
    WallRunTaskInstance->CurrentWallNormal = StartWallNormal;

    //필요 변수 초기화
    UDJGameplayAbility* DJGA = Cast<UDJGameplayAbility>(OwningAbility);
    if (DJGA == nullptr)
    {
        WallRunTaskInstance->OnDestroy(false);
        return nullptr;
    }

    WallRunTaskInstance->Character = DJGA->GetDJCharacterFromActorInfo();
    if (WallRunTaskInstance->Character == nullptr)
    {
        WallRunTaskInstance->OnDestroy(false);
        return nullptr;
    }
    WallRunTaskInstance->AnimInstance = Cast<UDJAnimInstance>(WallRunTaskInstance->Character->GetMesh()->GetAnimInstance());
    if (WallRunTaskInstance->AnimInstance == nullptr)
    {
        WallRunTaskInstance->OnDestroy(false);
        return nullptr;
    }

    
    if (UKismetSystemLibrary::DoesImplementInterface(WallRunTaskInstance->AnimInstance, UAnimSourceSetInterface::StaticClass()))
    {
        WallRunTaskInstance->AnimSourceSetInterface = WallRunTaskInstance->AnimInstance;
    }
    else
    {
        WallRunTaskInstance->OnDestroy(false);
        return nullptr;
    }

    if (WallRunTaskInstance->AnimSourceSetInterface == nullptr)
    {
        WallRunTaskInstance->OnDestroy(false);
        return nullptr;
    }
    
    WallRunTaskInstance->MaxSpeed = WallRunTaskInstance->Character->GetCharacterMovement()->GetMaxSpeed();

	return WallRunTaskInstance;
}

void UDJAT_WallRun::Activate()
{
    //Super::Activate();
    //처음 실행 시, 아바타에 WALLRUN 태그 부여 (애니메이션 실행 위함)
    AbilitySystemComponent.Get()->AddLooseGameplayTag(Tag_Status_Parkour_WallRun);

    //초기 스피드 가져오기
    FVector CurrentVelocity2D = Character->GetCharacterMovement()->Velocity;
    CurrentVirSpeed = FMath::Min( CurrentVelocity2D.Z +100.0, 200.0); //수직 스피드 (+상승보정)
    CurrentVelocity2D.Z = 0;
    CurrentSpeed = FMath::Min(FMath::Max(CurrentVelocity2D.Length()+150, 450.0), Character->GetCharacterMovement()->GetMaxSpeed());  //수평 스피드 (최소 450, 최대 maxSpeed)
    Character->GetCharacterMovement()->Velocity = FVector::ZeroVector;
    //UE_LOG(LogTemp, Log, TEXT("MaxSpeed : %f,  CurrentSpeed : %f"), Character->GetCharacterMovement()->GetMaxSpeed(), CurrentSpeed);
   
    //아바타 시작 방향 초기화
    int32 LeftRight = bLeft ? -1 : 1;
    FVector ForwardWithWall = FVector::UpVector.Cross(CurrentWallNormal) * LeftRight;   //벽에 대한 앞 방향 설정
    Character->SetActorRotation(FRotationMatrix::MakeFromX(ForwardWithWall).Rotator());
    Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
    AnimSourceSetInterface->SetDirectionWall(bLeft);    //애님인스턴스에 벽 방향 전달
    Character->GetCharacterMovement()->bUseControllerDesiredRotation = false;  
    BeforePosition2D = FVector::ZeroVector;
    //Character->GetCapsuleComponent()->SetCapsuleRadius(10);
}

void UDJAT_WallRun::OnDestroy(bool bInOwnerFinished)
{
    if (Character)
    {
        Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
    }
    AbilitySystemComponent.Get()->RemoveLooseGameplayTag(Tag_Status_Parkour_WallRun);
    Character->GetCharacterMovement()->bUseControllerDesiredRotation = true;

    Super::OnDestroy(bInOwnerFinished);
}

void UDJAT_WallRun::MontageBeginNotifyCallback(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
    if (NotifyName.Compare(FName("")) == 0)
    {
        ;
    }
}

void UDJAT_WallRun::OnCompleted()
{
    Completed.Broadcast();
    EndTask();
}

void UDJAT_WallRun::OnCanceled()
{
    Canceled.Broadcast();
    EndTask();
}

