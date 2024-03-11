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

	//~ UAnimInstance �Լ� ������
	void NativeInitializeAnimation() override;
	virtual void NativeBeginPlay() override;
	UFUNCTION(Meta = (BlueprintThreadSafe))
	void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	//~ ��

	virtual void InitializeWithAbilitySystem(UAbilitySystemComponent* ASC);

	//~ UAnimSourceSetInterface ������
	virtual void SetDirectionWall(uint8 bLeft) override { bLeftWall = bLeft; }
	virtual void SetDisplacementSpeedWithWall(float speed) override { DisplacementSpeedWithWall = speed; }
	virtual void SetGroundDistanceByWallRun(float distance) override { GroundDistanceByWallRun = distance; }
	//~ UAnimSourceSetInterface ��

protected:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif // WITH_EDITOR

protected:
	// Tag - Property ���ε�
	UPROPERTY(EditDefaultsOnly, Category = "GameplayTags")
	FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;

	//********* �ִ� �׷����� ������ ���� **********//
	//�ִ��ν��Ͻ��� ���� �����
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	TObjectPtr<class ACharacter> OwnerChar;
	//������ ĳ���� �����Ʈ �����*
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	TObjectPtr<class UDJCharacterMovementComponent> Movement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Setting)
	float LocalDirDeadZone;	//�̵� ���� ��꿡 ���� ���� ��������� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Setting)
	FVector2D YawOffestClamp;	//YawOffset ��� �Ӱ谪
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Setting)
	FVector2D YawOffestClampInCrouch;	//YawOffset Crouch�ϋ� ��� �Ӱ谪

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = LocationState)
	FVector WorldLocation;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = LocationState)
	float displacementLength;	//������ �� �̵��Ÿ� (2d)
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = LocationState)
	float displacementSpeed;	//�̵��ӵ� (2d)

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = VelocityState)
	uint8 bHasAcceleration : 1;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = VelocityState)
	uint8 bHasVelocity : 1;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = VelocityState)
	float GroundSpeed;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = VelocityState)
	float TimeToJumpApex;	// ���� �ְ������� ���� �ð�
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
	EMoveDir LocalDirWithYawOffset;	//YawOffset ���� ���� ���� ���� (velocity�� ���)
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadWrite, Category = DirState, Meta = (AllowPrivateAccess = "true"))
	EMoveDir StartDirWithYawOffset;	//Start ���� ���� �� ���� ����
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadWrite, Category = DirState, Meta = (AllowPrivateAccess = "true"))
	EMoveDir PivotDirWithYawOffset;//Pivot ���� ���� �� ���� ����
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = DirState)
	float LocalDirAngle;	//���� ������ ������ ��ȯ�� ��
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = DirState)
	float LocalDirAngleWithYawOffset;	//YawOffset ���� ���� ���� ���� ����

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = DirState)
	EMoveDir LocalAccelDirWithYawOffset;	//YawOffset ���� ���ð��� ����
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = DirState)
	float LocalAccelDirAngle;	//���� ���ӵ� ���� ����
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = DirState)
	float LocalAccelDirAngleWithYawOffset;	//YawOffset ���� ���� ���ӵ� ���� ����

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = CharacterState)
	uint8 bIsCrouch : 1;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = CharacterState)
	uint8 bCrouchChange : 1;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = CharacterState)
	uint8 bIsJump : 1;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = CharacterState)
	uint8 bIsFall : 1;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = CharacterState)
	uint8 bIsADS : 1;	//����(����) ����
	UPROPERTY(BlueprintReadOnly, Category = "CharacterState")
	float GroundDistance = -1.0f;	//(���� ��) �� ���� ���� �Ÿ�

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = MotionState)
	EMoveState MoveState;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadWrite, Category = AmingState)
	float AimYaw;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadWrite, Category = AmingState)
	float AimPitch;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadWrite, Category = YawState)
	EYawOffsetMode YawOffsetMode;	
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = YawState)
	float YawDelta;	//������ �� ī�޶��� Yaw ��ȭ
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadWrite, Category = YawState)
	float YawOffset;	//���� ĳ���Ͱ� �ٶ� ������ ���� ( ī�޶� Yaw ȸ�� �ݴ� ����). ��, ī�޶� ȸ���ص� ĳ���ʹ� ���� ����
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = YawState)
	float YawSpeed;	//ī�޶� Yaw ȸ�� ���ǵ� (���ӵ�) (additive Lean �ִϸ��̼� ������ �̿�)


	float BeforeYaw;				// c++ ������ ���
	struct FFloatSpringState YawOffsetSpringState;	// c++ ������ ( ������ ������ ���)

	//�� �ִϸ��̼ǿ��� ���� Ŀ�� ��(���� ȸ�� ��)
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadWrite, Category = YawState)
	float CurrentTurnCurveValue;

	//for WallRun
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = LocationState)
	float DisplacementSpeedWithWall;	//WallRun �̵��ӵ�
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = LocationState)
	uint8 bLeftWall :1 ;	//�� ����
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = LocationState)
	float GroundDistanceByWallRun;	//�� ����

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
