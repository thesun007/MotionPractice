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
	//캡슐 충돌 프로파일 변경
	//GetCapsuleComponent()->SetCollisionProfileName(CPROFILE_ABCAPSULE);

	//컨트롤러 회전과 동기화 안함.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// 이동 설정 초기화
	GetCharacterMovement()->bOrientRotationToMovement = false; 	// 캐릭터가 이동했을 때, 이동하는 방향으로 회전	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.2f;			//공중에 있을 때, 컨트롤 강도
	//GetCharacterMovement()->MaxWalkSpeed = Cast<UDJCharacterMovementComponent>(GetCharacterMovement())->GetMaxJogSpeed();
	GetCharacterMovement()->MaxAcceleration = 1024;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingFriction = 1.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 1000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 100.f;
	GetCharacterMovement()->SetCrouchedHalfHeight(65.f);
	GetCharacterMovement()->MaxWalkSpeedCrouched = 250.f;

	// Mesh 초기 설정
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -90.0f), FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	//매시 충돌 프로파일 변경
	//GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));
	
	//모션 워핑 생성
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));

	//마네킹 메시 에셋 등록. 메시는 오브젝트로서 가져옴.
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> CharacterMeshRef(TEXT("/Script/Engine.SkeletalMesh'/Game/Characters/Heroes/Mannequin/Meshes/SKM_Manny.SKM_Manny'"));
	if (CharacterMeshRef.Object)
	{
		GetMesh()->SetSkeletalMesh(CharacterMeshRef.Object);
	}

	
	//애니메이션은 블루프린트를 가져와서 클래스로 등록
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
	//	const FRotator CamRotation = Controller->GetControlRotation();	// 컨트롤 각도 = 카메라 방향+
	//	const FRotator CamYawRotation(0, CamRotation.Yaw, 0);

	//	//비스듬 회전 (카메라 방향에서 이동할 방향쪽 각도 차)
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

