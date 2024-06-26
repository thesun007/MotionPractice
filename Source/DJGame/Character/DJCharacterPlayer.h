// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/DJCharacterBase.h"
#include "Tag/DJGameplayTags.h"
#include "DJCharacterPlayer.generated.h"

struct FGameplayTag;
struct FInputActionValue;

DECLARE_DYNAMIC_DELEGATE_OneParam(FInputFunc, const FInputActionValue&, inputvalue);

/**
 * 
 */
UCLASS()
class DJGAME_API ADJCharacterPlayer : public ADJCharacterBase
{
	GENERATED_BODY()
	
public:
	ADJCharacterPlayer(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "DJ|Character")
	class ADJPlayerController* GetDJPlayerController() const;

	UFUNCTION(BlueprintCallable, Category = "DJ|Character")
	class ADJPlayerState* GetDJPlayerState() const;

	UFUNCTION(BlueprintCallable, Category = "DJ|Character")
	FGameplayTagContainer& GetMoveInputBlockTag() {return MoveInputBlockTag;}

	
	void ResetMove();

public:
	UPROPERTY(BlueprintReadWrite, Category = "Callback")
	FInputFunc MoveFuncRef;		// 무브 

protected:
	//~ AActor 재정의
	virtual void BeginPlay() override;
	//~ 끝 

	//~ Pawn 재정의
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	//~ Pawn 끝 

	//~ <input - tag - ability> 로직
	virtual bool CanJumpInternal_Implementation() const override;
	virtual void Jump() override;

	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);

	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_LookMouse(const FInputActionValue& InputActionValue);
	//void Input_LookStick(const FInputActionValue& InputActionValue);
	void Input_Crouch(const FInputActionValue& InputActionValue);
	//void Input_AutoRun(const FInputActionValue& InputActionValue);

private:
	virtual void InitializeASC();
	UFUNCTION()
	void DoMove(const FInputActionValue& InputActionValue);
private:

	//카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UDJCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	uint8 bMovementInputLock :1 ;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "DJ|Character", Meta = (AllowPrivateAccess = "true"))
	FGameplayTagContainer MoveInputBlockTag;
};
