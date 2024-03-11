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
	//******************* ī�޶� ���� ******************//
	//������ �� ������Ʈ ����, ��Ʈ ����
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 250.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bInheritPitch = true;
	CameraBoom->bInheritYaw = true;
	CameraBoom->bInheritRoll = true;
	CameraBoom->bDoCollisionTest = true;

	//ī�޶� ���� ��, ������ �Ͽ� ����
	FollowCamera = CreateDefaultSubobject<UDJCameraComponent>(TEXT("FollowCamera"));
	// �����̸����� Ư����ġ�� ����
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);		//�⺻ "SpringEndpoint"���� ������.
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

	// Local Controller�� ��, PostInitial -> ���� -> BeginPlay()
	// ���� BeginPlay() ���Ŀ� ��Ʈ�ѷ��� ������ ���� ���ؽ�Ʈ �۾�
	FTimerHandle handle2;
	GetWorld()->GetTimerManager().SetTimer(handle2, FTimerDelegate::CreateLambda(
		[&]()
		{
			/*FVector accel = GetCharacterMovement()->GetCurrentAcceleration();
			FVector velo = GetCharacterMovement()->Velocity;
			UE_LOG(LogTemp, Log, TEXT("accel [%f, %f, %f] / velocity [%f, %f, %f] / speed [%f]"), accel.X, accel.Y, accel.Z, velo.X, velo.Y, velo.Z, velo.Length());*/

			//���� ���� ����� ������ ������
			//FVector StartPos = GetActorLocation();   //ĳ���� ��ġ
			//FVector EndPos = StartPos - GetActorUpVector() * 1000;   // �Ʒ� ���� ��ġ

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

		//���ǵ�����, ĳ���Ϳ� PawnData�� ����
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

		//���ǵ�����, ĳ���Ϳ� PawnData�� ����
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
	//������Ʈ ���ÿ��� ������ ���� ��ǲ ������Ʈ Ŭ������ ���� �޾� �̺�Ʈ ���ε� �� �� �ֵ��� �ϴ� �Լ�
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//������ ���� ���ؽ�Ʈ ���� �� ��ǲ �ý��ۿ� �߰��ϴ� �۾�
	APlayerController* PlayerController = CastChecked<APlayerController>(GetController());
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		//����� �Է� ���� ���ؽ�Ʈ ��� ����
		Subsystem->ClearAllMappings(); 
		//PlayerState���� PawnData �����ͼ� Input ���ε�
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
		//���� ���̶�� Ű�Է� ���� ����
		FGameplayTagContainer tags;
		ASC->GetOwnedGameplayTags(tags);
		if(tags.IsEmpty() == false && tags.HasAnyExact(MoveInputBlockTag))
		{
			return;
		}

		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();	// ��Ʈ�� ���� = ī�޶� ����
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// ī�޶�(��Ʈ��) ���� forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);	//���� �� ����.

		// ī�޶�(��Ʈ��) ���� right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.X);
		AddMovementInput(RightDirection, MovementVector.Y);

	}
}

void ADJCharacterPlayer::Input_LookMouse(const FInputActionValue& InputActionValue)
{
	//���콺 �Է� ����
	FVector2D LookAxisVector = InputActionValue.Get<FVector2D>() * 0.4;

	//��Ʈ�ѷ��� ControlRotation �Ӽ��� ������Ʈ�Ѵ�.
	//�ᱹ ī�޶� ȸ�� (�ֳ��ϸ� ������ ���� ��Ʈ�ѷ� �����̼� �����)
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