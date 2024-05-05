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

        //��ֹ� ���� Ȯ�� ����
        FVector ActorPos2D = Character->GetActorLocation(); 
        ActorPos2D.Z = 0;
        if ((BeforePosition2D - ActorPos2D).SquaredLength() < 0.5f)    //�̵��� ���� ��������..
        {
            UE_LOG(LogTemp, Log, TEXT("on Blocking"));
            OnCanceled();
            return;
        }
        BeforePosition2D = ActorPos2D;
        
        /*���� ��ġ ���*/
        //���� ���� ��� (�� ������ �̵�)
        CurrentVirSpeed += (Character->GetCharacterMovement()->GetGravityZ()*0.4f ) * DeltaTime;    //�߷�(1/2.5) ����
        float DeltaZ = CurrentVirSpeed * DeltaTime; //���� Z ��
        
        //�������� �ӵ��� �������� ����
        float FallAngle = FMath::RadiansToDegrees(FMath::Atan2(CurrentVirSpeed, CurrentSpeed));
        if (FallAngle < -45)  //�������� ������ -45����
        {
            OnCanceled();
            return;
        }

        //���� ���� ��ġ ���
        FVector StartPos = Character->GetActorLocation() + Character->GetActorForwardVector() * CurrentSpeed * DeltaTime;   //ĳ���� ���� ���� ��ġ
        StartPos.Z += DeltaZ;
        FVector EndPos = StartPos + Character->GetActorRightVector()* LeftRight * (GapOffsetWithWall + 5);   // ���� ������ġ���� �� ������ LineTrace

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

        if (Hit == false)   //���� ������ ����
        {
            OnCanceled();
            return;
        }

        CurrentWallNormal = HitResult.ImpactNormal;
        FVector FinalPos = HitResult.ImpactPoint + HitResult.ImpactNormal * GapOffsetWithWall;  //���� ��ġ
        FVector ForwardWithWall = FVector::UpVector.Cross(CurrentWallNormal) * LeftRight;
        FRotator FinalRot = FRotationMatrix::MakeFromX(ForwardWithWall).Rotator();
        
        FRotator CheckRotDir = FinalRot - Character->GetActorRotation(); CheckRotDir.Normalize();   //ȸ���ؾ��� ���� Ȯ�� (���� �ٱ����� �־��������� ���)
        //UE_LOG(LogTemp, Log, TEXT("CheckRot : %f"), CheckRotDir.Yaw * LeftRight);
        if (CheckRotDir.Yaw * LeftRight > 1.5f)
        {
            OnCanceled();
            return;
        }

        //ĳ���� ���� Ʈ������ ����
        Character->SetActorLocationAndRotation(FinalPos, FinalRot);

        //�� ���� �̵��� ����ӵ� �ִϸ��̼ǿ� ����
        FVector Displacement2D = HitResult.ImpactPoint - CurrentWallPosition;   // (�������� ����̶�� CurrentSpeed�� �޶��� ��)
        Displacement2D.Z = 0;
        AnimSourceSetInterface->SetDisplacementSpeedWithWall(Displacement2D.Length() / DeltaTime); 
        CurrentWallPosition = HitResult.ImpactPoint;    //�� ����Ʈ ��ġ ����
        //UE_LOG(LogTemp, Log, TEXT("2D Speed : %f"), Displacement2D.Length() / DeltaTime);

        /* ���� ���� ��ġ üũ */
        FHitResult GroundHitResult;
        FCollisionQueryParams GroundQueryParams(SCENE_QUERY_STAT(CheckGround), false, Character);
        FVector Start = Character->GetActorLocation();
        FVector CheckDownDir = FVector(1, 0, -1);  //���� ���̾����� �� ������ �Ʒ� ���� 
        CheckDownDir.Normalize();
        CheckDownDir = FRotationMatrix(FRotator(0, Character->GetActorRotation().Yaw + 45 * -LeftRight, 0)).TransformVector(CheckDownDir);
        
        FVector End = Start + CheckDownDir *300;    //(��ǥ�� �����Ʒ� �Ÿ� 60���ϵǴ� Ÿ�̹��̶� ���� 100���� ����)

        Hit = GetWorld()->LineTraceSingleByObjectType(GroundHitResult, Start, End, FCollisionObjectQueryParams::AllStaticObjects, GroundQueryParams);   //���� üũ

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

        // �� �ٴ� ���� ������� �����Ʒ� ���̰� 60���ϸ� ���� �õ�
        if (FallAngle < -20 && Hit && GroundHitResult.ImpactNormal.Z > 0 &&
            (Start.Z - Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - GroundHitResult.ImpactPoint.Z) < 60) 
        {
            LandingPoint = GroundHitResult.ImpactPoint;
            RunState = EWallRunState::End;
            AbilitySystemComponent.Get()->RemoveLooseGameplayTag(Tag_Status_Parkour_WallRun);
        }
        else
        {
            AnimSourceSetInterface->SetGroundDistanceByWallRun(100);    //���� ���� ������ ���Ƿ� 100 ����
        }
    }
    else
    {
        int32 LeftRight = bLeft ? 1 : -1;
        
        /* wallrun ���� ��� �� ������ ����(����) */
        float GroundDistance = Character->GetActorLocation().Z - Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - LandingPoint.Z;   //�߹ٴں��� ���� �������� �����Ÿ� ����

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

    //�ʿ� ���� �ʱ�ȭ
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
    //ó�� ���� ��, �ƹ�Ÿ�� WALLRUN �±� �ο� (�ִϸ��̼� ���� ����)
    AbilitySystemComponent.Get()->AddLooseGameplayTag(Tag_Status_Parkour_WallRun);

    //�ʱ� ���ǵ� ��������
    FVector CurrentVelocity2D = Character->GetCharacterMovement()->Velocity;
    CurrentVirSpeed = FMath::Min( CurrentVelocity2D.Z +100.0, 200.0); //���� ���ǵ� (+��º���)
    CurrentVelocity2D.Z = 0;
    CurrentSpeed = FMath::Min(FMath::Max(CurrentVelocity2D.Length()+150, 450.0), Character->GetCharacterMovement()->GetMaxSpeed());  //���� ���ǵ� (�ּ� 450, �ִ� maxSpeed)
    Character->GetCharacterMovement()->Velocity = FVector::ZeroVector;
    //UE_LOG(LogTemp, Log, TEXT("MaxSpeed : %f,  CurrentSpeed : %f"), Character->GetCharacterMovement()->GetMaxSpeed(), CurrentSpeed);
   
    //�ƹ�Ÿ ���� ���� �ʱ�ȭ
    int32 LeftRight = bLeft ? -1 : 1;
    FVector ForwardWithWall = FVector::UpVector.Cross(CurrentWallNormal) * LeftRight;   //���� ���� �� ���� ����
    Character->SetActorRotation(FRotationMatrix::MakeFromX(ForwardWithWall).Rotator());
    Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
    AnimSourceSetInterface->SetDirectionWall(bLeft);    //�ִ��ν��Ͻ��� �� ���� ����
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

