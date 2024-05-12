// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/DJCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "Character/DJCharacterMovementComponent.h"
#include "Engine/AssetManager.h"
#include "MotionWarpingComponent.h"
#include "GAS/DJAbilitySystemComponent.h"

// Sets default values
ADJCharacterBase::ADJCharacterBase(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(40.f, 90.0f);
	//ĸ�� �浹 �������� ����
	//GetCapsuleComponent()->SetCollisionProfileName(CPROFILE_ABCAPSULE);

	//��Ʈ�ѷ� ȸ���� ����ȭ ����.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// �̵� ���� �ʱ�ȭ
	GetCharacterMovement()->bOrientRotationToMovement = false; 	// ĳ���Ͱ� �̵����� ��, �̵��ϴ� �������� ȸ��	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.2f;			//���߿� ���� ��, ��Ʈ�� ����
	//GetCharacterMovement()->MaxWalkSpeed = Cast<UDJCharacterMovementComponent>(GetCharacterMovement())->GetMaxJogSpeed();
	GetCharacterMovement()->MaxAcceleration = 1024;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingFriction = 1.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 1000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 100.f;
	GetCharacterMovement()->SetCrouchedHalfHeight(65.f);
	GetCharacterMovement()->MaxWalkSpeedCrouched = 250.f;

	// Mesh �ʱ� ����
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -90.0f), FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	//�Ž� �浹 �������� ����
	//GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));
	
	//��� ���� ����
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));

	//����ŷ �޽� ���� ���. �޽ô� ������Ʈ�μ� ������.
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> CharacterMeshRef(TEXT("/Script/Engine.SkeletalMesh'/Game/Characters/Heroes/Mannequin/Meshes/SKM_Manny.SKM_Manny'"));
	if (CharacterMeshRef.Object)
	{
		GetMesh()->SetSkeletalMesh(CharacterMeshRef.Object);
	}

	
	//�ִϸ��̼��� �������Ʈ�� �����ͼ� Ŭ������ ���
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimInstanceClassRef(TEXT("/Game/DJGame/Animations/ABP_DJCharacterMain.ABP_DJCharacterMain_C"));
	if (AnimInstanceClassRef.Class)
	{
		GetMesh()->SetAnimInstanceClass(AnimInstanceClassRef.Class);
	}
}

void ADJCharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	SetAnimLayer(EFullBodyPose::UnArmed);
}

void ADJCharacterBase::SetAnimLayer(EFullBodyPose pose)
{
	if(OtherAnimLayers.Find(pose))
		GetMesh()->GetAnimInstance()->LinkAnimClassLayers(OtherAnimLayers[pose]);
}

UAbilitySystemComponent* ADJCharacterBase::GetAbilitySystemComponent() const
{
	return GetDJAbilitySystemComponent();
}

void ADJCharacterBase::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
}

bool ADJCharacterBase::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
	return false;
}

bool ADJCharacterBase::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	return false;
}

bool ADJCharacterBase::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	return false;
}

// Called when the game starts or when spawned
void ADJCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADJCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); 

	//FRotator LastCamYawRotation;
	//if (Controller != nullptr && GetCharacterMovement()->GetCurrentAcceleration().Length()>0)
	//{
	//	// find out which way is forward
	//	const FRotator CamRotation = Controller->GetControlRotation();	// ��Ʈ�� ���� = ī�޶� ����+
	//	const FRotator CamYawRotation(0, CamRotation.Yaw, 0);

	//	//�񽺵� ȸ�� (ī�޶� ���⿡�� �̵��� ������ ���� ��)
	//	FRotator Diff = FRotationMatrix::MakeFromX(GetCharacterMovement()->GetCurrentAcceleration()).Rotator() - CamYawRotation;
	//	double DiffAngle = FRotator::NormalizeAxis(Diff.Yaw);
	//	double DiffAngle2 = 0.;
	
	//	if (-45 <= DiffAngle && DiffAngle < 45)
	//	{
	//		DiffAngle2 = DiffAngle;
	//	}

	//	else if (45 <= DiffAngle && DiffAngle < 135)
	//	{
	//		DiffAngle2 = 90 - DiffAngle;
	//	}
	//	else if (135 <= DiffAngle && DiffAngle < 180)
	//	{
	//		DiffAngle2 = DiffAngle - 180;
	//	}

	//	else if (-135 <= DiffAngle && DiffAngle < -45)
	//	{
	//		DiffAngle2 = -90 - DiffAngle;
	//	}
	//	else if (-180 <= DiffAngle && DiffAngle < -135)
	//	{
	//		DiffAngle2 = DiffAngle + 180;
	//	}

	//	FRotator CurrentRot = GetActorRotation();
	//	FRotator GoalRot = CamYawRotation + FRotator(0., DiffAngle2, 0.);

	//	SetActorRotation(FMath::RInterpTo(CurrentRot, GoalRot, DeltaTime, 10));
	//}
}

