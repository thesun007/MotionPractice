// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/DJCharacterPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "InputMappingContext.h"
#include "Input/DJInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "DJCharacterMovementComponent.h"
#include "Camera/DJCameraComponent.h"

#include "Player/DJPlayerController.h"
#include "Player/DJPlayerState.h"
#include "GAS/DJAbilitySystemComponent.h"
#include "PawnData.h"
#include "Input/DJInputData.h"
#include "GAS/DJAbilitySet.h"

#include "Animation/DJAnimInstance.h"

#include <type_traits>

ADJCharacterPlayer::ADJCharacterPlayer(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UDJCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	//******************* 카메라 설정 ******************//
	//스프링 암 컴포넌트 생성, 루트 설정
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 250.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bInheritPitch = true;
	CameraBoom->bInheritYaw = true;
	CameraBoom->bInheritRoll = true;
	CameraBoom->bDoCollisionTest = true;

	//카메라 생성 후, 스프링 암에 부착
	FollowCamera = CreateDefaultSubobject<UDJCameraComponent>(TEXT("FollowCamera"));
	// 소켓이름으로 특정위치에 부착
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);		//기본 "SpringEndpoint"으로 되있음.
	FollowCamera->bUsePawnControlRotation = false;
	FollowCamera->SetRelativeLocation(FVector(0, 30, 0));

	MoveInputBlockTag.AddTag(Tag_Status_Parkour_WallRun);
	//MoveInputBlockTag.AddTag(Tag_Status_Sliding);
}

ADJPlayerController* ADJCharacterPlayer::GetDJPlayerController() const
{
	return CastChecked<ADJPlayerController>(Controller, ECastCheckedType::NullAllowed);
}

ADJPlayerState* ADJCharacterPlayer::GetDJPlayerState() const
{
	return CastChecked<ADJPlayerState>(GetPlayerState(), ECastCheckedType::NullAllowed);
}

UDJAbilitySystemComponent* ADJCharacterPlayer::GetDJAbilitySystemComponent() const
{
	return ASC;
}

UAbilitySystemComponent* ADJCharacterPlayer::GetAbilitySystemComponent() const
{
	return ASC;
}

void ADJCharacterPlayer::BeginPlay()
{
	Super::BeginPlay();

	// Local Controller일 떄, PostInitial -> 빙의 -> BeginPlay()
	// 따라서 BeginPlay() 이후에 컨트롤러에 접근해 맵핑 컨텍스트 작업
	FTimerHandle handle2;
	GetWorld()->GetTimerManager().SetTimer(handle2, FTimerDelegate::CreateLambda(
		[&]()
		{
			/*FVector accel = GetCharacterMovement()->GetCurrentAcceleration();
			FVector velo = GetCharacterMovement()->Velocity;
			UE_LOG(LogTemp, Log, TEXT("accel [%f, %f, %f] / velocity [%f, %f, %f] / speed [%f]"), accel.X, accel.Y, accel.Z, velo.X, velo.Y, velo.Z, velo.Length());*/

			//현재 지면 기울기와 정면의 각도차
			//FVector StartPos = GetActorLocation();   //캐릭터 위치
			//FVector EndPos = StartPos - GetActorUpVector() * 1000;   // 아래 방향 위치

			//FHitResult HitResult;
			//FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CheckWall), false, this);
			//bool Hit = GetWorld()->LineTraceSingleByObjectType(HitResult, StartPos, EndPos, FCollisionObjectQueryParams::AllStaticObjects, QueryParams);
			//if (Hit)
			//{
			//	UE_LOG(LogTemp, Log, TEXT("ground dot : %f"), HitResult.ImpactNormal.Dot(GetActorForwardVector()));
			//}

		}
	), 0.016f, true, -1.0f);
}

void ADJCharacterPlayer::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	ADJPlayerState* OwnerPS = GetPlayerState<ADJPlayerState>();
	if (OwnerPS)
	{
		ASC = OwnerPS->GetDJAbilitySystemComponent();
		ASC->InitAbilityActorInfo(OwnerPS, this);

		UDJAnimInstance* AnimInstance = Cast<UDJAnimInstance>(GetMesh()->GetAnimInstance());
		if (AnimInstance)
		{
			AnimInstance->InitializeWithAbilitySystem(ASC);
		}

		//빙의됐을때, 캐릭터에 PawnData를 적용
		const UPawnData* PawnData = GetDJPlayerState()->GetPawnData<UPawnData>();
		for (const UDJAbilitySet* AbilitySet : PawnData->AbilitySets)
		{
			if (AbilitySet)
			{
				AbilitySet->GiveToAbilitySystem(ASC, nullptr, this);
			}
		}
	}
}

void ADJCharacterPlayer::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	ADJPlayerState* OwnerPS = GetPlayerState<ADJPlayerState>();
	if (OwnerPS)
	{
		ASC = OwnerPS->GetDJAbilitySystemComponent();
		ASC->InitAbilityActorInfo(OwnerPS, this);

		UDJAnimInstance* AnimInstance = Cast<UDJAnimInstance>(GetMesh()->GetAnimInstance());
		if (AnimInstance)
		{
			AnimInstance->InitializeWithAbilitySystem(ASC);
		}

		//빙의됐을때, 캐릭터에 PawnData를 적용
		const UPawnData* PawnData = GetDJPlayerState()->GetPawnData<UPawnData>();
		for (const UDJAbilitySet* AbilitySet : PawnData->AbilitySets)
		{
			if (AbilitySet)
			{
				AbilitySet->GiveToAbilitySystem(ASC, nullptr, this);
			}
		}
	}

}

void ADJCharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	//프로젝트 세팅에서 설정한 향상된 인풋 컴포넌트 클래스를 참조 받아 이벤트 바인딩 할 수 있도록 하는 함수
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//가져온 맵핑 컨텍스트 에셋 을 인풋 시스템에 추가하는 작업
	APlayerController* PlayerController = CastChecked<APlayerController>(GetController());
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		//적용된 입력 맵핑 컨텍스트 모두 제거
		Subsystem->ClearAllMappings(); 
		//PlayerState에서 PawnData 가져와서 Input 바인딩
		const UPawnData* PawnData =  GetDJPlayerState()->GetPawnData<UPawnData>();
		if (PawnData)
		{
			if (const UDJInputData* InputData = PawnData->InputData)
			{
				for (const FInputMappingContextAndPriority& Mapping : PawnData->InputMappings)
				{
					Mapping.InputMapping.LoadSynchronous();
					if (UInputMappingContext* IMC = Mapping.InputMapping.Get())
					{
						//if (Mapping.bRegisterWithSettings)
						//{
						//	if (UEnhancedInputUserSettings* Settings = Subsystem->GetUserSettings())
						//	{
						//		Settings->RegisterInputMappingContext(IMC);
						//	}

							FModifyContextOptions Options = {};
							Options.bIgnoreAllPressedKeysUntilRelease = false;
							// Actually add the config to the local player							
							Subsystem->AddMappingContext(IMC, Mapping.Priority, Options);
						//}
					}
				}

				//******** bind InputAction ************//
				UDJInputComponent* DJIC = Cast<UDJInputComponent>(PlayerInputComponent);
				
				if (ensureMsgf(DJIC, TEXT("Unexpected Input Component class! The Gameplay Abilities will not be bound to their inputs. Change the input component to UDJInputComponent or a subclass of it.")))
				{
					// Add the key mappings that may have been set by the player
					DJIC->AddInputMappings(InputData, Subsystem);

					// This is where we actually bind and input action to a gameplay tag, which means that Gameplay Ability Blueprints will
					// be triggered directly by these input actions Triggered events. 
					TArray<uint32> BindHandles;
					DJIC->BindAbilityActions(InputData, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);
					
					DJIC->BindNativeAction(InputData, InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, /*bLogIfNotFound=*/ false);
					DJIC->BindNativeAction(InputData, InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse, /*bLogIfNotFound=*/ false);
					//DJIC->BindNativeAction(InputData, InputTag_Look_Stick, ETriggerEvent::Triggered, this, &ThisClass::Input_LookStick, /*bLogIfNotFound=*/ false);
					DJIC->BindNativeAction(InputData, InputTag_Crouch, ETriggerEvent::Triggered, this, &ThisClass::Input_Crouch, /*bLogIfNotFound=*/ false);
					DJIC->BindNativeAction(InputData, INPUTTAG_JUMP, ETriggerEvent::Triggered, this, &ThisClass::Jump, /*bLogIfNotFound=*/ false);
					DJIC->BindNativeAction(InputData, INPUTTAG_JUMP, ETriggerEvent::Completed, this, &ThisClass::StopJumping, /*bLogIfNotFound=*/ false);
					//DJIC->BindNativeAction(InputData, InputTag_AutoRun, ETriggerEvent::Triggered, this, &ThisClass::Input_AutoRun, /*bLogIfNotFound=*/ false);
				}
			}
		}
	}
}

bool ADJCharacterPlayer::CanJumpInternal_Implementation() const
{
	//bool ParentResult =  Super::CanJumpInternal_Implementation();
	
	return JumpIsAllowedInternal();

	/*if (ParentResult)
		return ParentResult;
	else
		return !GetCharacterMovement()->IsFalling() && GetCharacterMovement()->IsCrouching();*/

}

void ADJCharacterPlayer::Jump()
{
	UnCrouch();
	Super::Jump();
}

void ADJCharacterPlayer::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (ASC)
	{
		ASC->AbilityInputTagPressed(InputTag);
	}
}

void ADJCharacterPlayer::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (ASC)
	{
		ASC->AbilityInputTagReleased(InputTag);
	}
}

void ADJCharacterPlayer::Input_Move(const FInputActionValue& InputActionValue)
{
	// input is a Vector2D
	FVector2D MovementVector = InputActionValue.Get<FVector2D>();

	if (Controller != nullptr)
	{
		//파쿠르 중이라면 키입력 무빙 무시
		FGameplayTagContainer tags;
		ASC->GetOwnedGameplayTags(tags);
		if(tags.IsEmpty() == false && tags.HasAnyExact(MoveInputBlockTag))
		{
			return;
		}

		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();	// 컨트롤 각도 = 카메라 방향
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// 카메라(컨트롤) 기준 forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);	//현재 앞 방향.

		// 카메라(컨트롤) 기준 right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.X);
		AddMovementInput(RightDirection, MovementVector.Y);

	}
}

void ADJCharacterPlayer::Input_LookMouse(const FInputActionValue& InputActionValue)
{
	//마우스 입력 같음
	FVector2D LookAxisVector = InputActionValue.Get<FVector2D>() * 0.4;

	//컨트롤러의 ControlRotation 속성을 업데이트한다.
	//결국 카메라 회전 (왜냐하면 스프링 암이 컨트롤러 로테이션 사용함)
	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void ADJCharacterPlayer::Input_Crouch(const FInputActionValue& InputActionValue)
{
	if (GetCharacterMovement()->IsFalling())
		return;

	if (GetCharacterMovement()->IsCrouching())
	{
		// If currently crouched, uncrouch
		UnCrouch();
	}
	else
	{
		// If not crouched, crouch
		Crouch();
	}
}