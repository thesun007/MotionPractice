// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Interface/MainAnimInterface.h"
#include "Interface/AnimSourceSetInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameplayEffectTypes.h"
#include "DJAnimInstance.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EMoveState : uint8
{
	idle,
	move,
	stop,
	pivot
};

UENUM(BlueprintType)
enum class EMoveDir : uint8
{
	Forward,
	Backward,
	Left,
	Right
};

UENUM(BlueprintType)
enum class EYawOffsetMode : uint8
{
	Execute,
	BlendOut,
	Hold
};

UCLASS()
class DJGAME_API UDJAnimInstance : public UAnimInstance, public IMainAnimInterface, public IAnimSourceSetInterface
{
	GENERATED_BODY()
	
public:
	UDJAnimInstance();

	//~ UAnimInstance 함수 재정의
	void NativeInitializeAnimation() override;
	virtual void NativeBeginPlay() override;
	UFUNCTION(Meta = (BlueprintThreadSafe))
	void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	//~ 끝

	virtual void InitializeWithAbilitySystem(UAbilitySystemComponent* ASC);

	//~ UAnimSourceSetInterface 재정의
	virtual void SetDirectionWall(uint8 bLeft) override { bLeftWall = bLeft; }
	virtual void SetDisplacementSpeedWithWall(float speed) override { DisplacementSpeedWithWall = speed; }
	virtual void SetGroundDistanceByWallRun(float distance) override { GroundDistanceByWallRun = distance; }
	//~ UAnimSourceSetInterface 끝

protected:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif // WITH_EDITOR

protected:
	// Tag - Property 바인드
	UPROPERTY(EditDefaultsOnly, Category = "GameplayTags")
	FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;

	//********* 애님 그래프가 참조할 변수 **********//
	//애님인스턴스의 오너 저장용
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	TObjectPtr<class ACharacter> OwnerChar;
	//오너의 캐릭터 무브먼트 저장용*
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	TObjectPtr<class UDJCharacterMovementComponent> Movement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Setting)
	float LocalDirDeadZone;	//이동 방향 계산에 가변 범위 적용용으로 쓰임
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Setting)
	FVector2D YawOffestClamp;	//YawOffset 허용 임계값
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Setting)
	FVector2D YawOffestClampInCrouch;	//YawOffset Crouch일떄 허용 임계값

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = LocationState)
	FVector WorldLocation;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = LocationState)
	float displacementLength;	//프레임 당 이동거리 (2d)
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = LocationState)
	float displacementSpeed;	//이동속도 (2d)

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = VelocityState)
	uint8 bHasAcceleration : 1;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = VelocityState)
	uint8 bHasVelocity : 1;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = VelocityState)
	float GroundSpeed;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = VelocityState)
	float TimeToJumpApex;	// 점프 최고점까지 남은 시간
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = VelocityState)
	FVector WorldVelocity;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = VelocityState)
	FVector WorldAccel;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = VelocityState)
	FVector WorldVelocity_2D;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = VelocityState)
	FVector LocalVelocity2D;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = VelocityState)
	FVector WorldAccel_2D;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = VelocityState)
	FVector LocalAccel2D;
	
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = DirState, Meta = (AllowPrivateAccess = "true"))
	EMoveDir LocalDirWithYawOffset;	//YawOffset 각도 기준 로컬 방향 (velocity로 계산)
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadWrite, Category = DirState, Meta = (AllowPrivateAccess = "true"))
	EMoveDir StartDirWithYawOffset;	//Start 상태 진입 시 로컬 방향
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadWrite, Category = DirState, Meta = (AllowPrivateAccess = "true"))
	EMoveDir PivotDirWithYawOffset;//Pivot 상태 진입 시 로컬 방향
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = DirState)
	float LocalDirAngle;	//로컬 방향을 각도로 변환한 것
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = DirState)
	float LocalDirAngleWithYawOffset;	//YawOffset 각도 기준 로컬 방향 각도

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = DirState)
	EMoveDir LocalAccelDirWithYawOffset;	//YawOffset 기준 로컬가속 방향
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = DirState)
	float LocalAccelDirAngle;	//로컬 가속도 방향 각도
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = DirState)
	float LocalAccelDirAngleWithYawOffset;	//YawOffset 기준 로컬 가속도 방향 각도

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = CharacterState)
	uint8 bIsCrouch : 1;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = CharacterState)
	uint8 bCrouchChange : 1;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = CharacterState)
	uint8 bIsJump : 1;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = CharacterState)
	uint8 bIsFall : 1;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = CharacterState)
	uint8 bIsADS : 1;	//조준(견착) 여부
	UPROPERTY(BlueprintReadOnly, Category = "CharacterState")
	float GroundDistance = -1.0f;	//(점프 시) 땅 까지 남은 거리

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = MotionState)
	EMoveState MoveState;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadWrite, Category = AmingState)
	float AimYaw;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadWrite, Category = AmingState)
	float AimPitch;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadWrite, Category = YawState)
	EYawOffsetMode YawOffsetMode;	
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = YawState)
	float YawDelta;	//프레임 당 카메라의 Yaw 변화
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadWrite, Category = YawState)
	float YawOffset;	//실제 캐릭터가 바라볼 방향의 각도 ( 카메라 Yaw 회전 반대 각도). 즉, 카메라 회전해도 캐릭터는 각도 유지
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = YawState)
	float YawSpeed;	//카메라 Yaw 회전 스피드 (각속도) (additive Lean 애니메이션 각도로 이용)


	float BeforeYaw;				// c++ 에서만 사용
	struct FFloatSpringState YawOffsetSpringState;	// c++ 에서만 ( 스프링 보간에 사용)

	//턴 애니메이션에서 현재 커브 값(남은 회전 값)
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadWrite, Category = YawState)
	float CurrentTurnCurveValue;

	//for WallRun
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = LocationState)
	float DisplacementSpeedWithWall;	//WallRun 이동속도
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = LocationState)
	uint8 bLeftWall :1 ;	//벽 방향
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = LocationState)
	float GroundDistanceByWallRun;	//벽 방향

protected:
	void UpdateLocation(float DeltaSeconds);
	void UpdateVelocity();
	EMoveDir UpdateDir(float localAngle, float DirDeadZone, EMoveDir BeforeLocalDir, bool WasMovingLastUpdate);
	void UpdateState(bool WasMovingLastUpdate);
	void ReverseDir(EMoveDir& TargetDir);
	void UpdateYawOffset(float DeltaSeconds);
	//UFUNCTION(BlueprintCallable, Category = YawState, Meta = (AllowPrivateAccess = "true", BlueprintThreadSafe))
	void SetYawOffset(float inYawOffset);

	UFUNCTION(BlueprintCallable, Category = TurnState, Meta = (AllowPrivateAccess = "true", BlueprintThreadSafe))
	void ProcessTurnCurve()	;

public:
	virtual UDJAnimInstance* GetMainAnimIns() const override;
};
