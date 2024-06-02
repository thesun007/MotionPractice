// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "EnhancedInputComponent.h"
#include "Input/DJInputData.h"
#include "DJGame.h"
#include "DJGameplayAbility.generated.h"


UENUM(BlueprintType)
enum class EDJAbilityActivationPolicy : uint8
{
	// Try to activate the ability when the input is triggered. (인풋 시, 처음엔 ActivateAbility(). 살아있다면 다음부턴 InputPressed())
	OnInputTriggered,

	// Continually try to activate the ability while the input is active. (인풋 이후에, 지속적으로 ActivateAbility() 실행됨.)
	WhileInputActive,

	// Try to activate the ability when an avatar is assigned. (스폰 시, 미리 ActivateAbility() 적용, 이후 살아 있다면 입력시 InputPressed())
	OnSpawn
};

/**
 * 
 */
UCLASS(Abstract, Meta = (ShortTooltip = "The base gameplay ability class used by this project."))
class DJGAME_API UDJGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	friend class ULyraAbilitySystemComponent;

public:
	UDJGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UFUNCTION(BlueprintCallable, Category = "DJ|Ability")
	class UDJAbilitySystemComponent* GetDJAbilitySystemComponentFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "DJ|Ability")
	class ADJPlayerController* GetDJPlayerControllerFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Lyra|Ability")
	AController* GetControllerFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "DJ|Ability")
	class ADJCharacterBase* GetDJCharacterFromActorInfo() const;

	EDJAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }
	//ELyraAbilityActivationGroup GetActivationGroup() const { return ActivationGroup; }

	//수동 혹은 OnGiveAbility() 할때 시도
	void TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const;

protected:
	//~UGameplayAbility interface
	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) override;
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~End

	/** Called when this ability is granted to the ability system component. */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityAdded")
	void K2_OnAbilityAdded();

	//사용자 입력 바인딩 재정의용
	virtual void SetInputAction();
	template<class UserClass, typename FuncType>
	void BindNativeAction(const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogNotFound);

private:
	// 입력 설정(Commit 에서 호출 됨)
	void InitInputMapping();
	// 입력 제거 (EndAbility 에서 호출 됨)
	void RemoveInputMapping();

protected:
	// Defines how this ability is meant to activate.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Ability Activation")
	EDJAbilityActivationPolicy ActivationPolicy;

private:
	//* 입력 섹션 *//
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DJ|Input", meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<class UInputMappingContext> InputMapping;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DJ|Input", meta = (AllowPrivateAccess = "true"))
	TArray<FDJInputAction> NativeInputActions;
	TArray<uint32> InputHandles;
	UPROPERTY()
	TObjectPtr<UEnhancedInputComponent> InputComponent;
};

template<class UserClass, typename FuncType>
void UDJGameplayAbility::BindNativeAction(const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogNotFound)
{
	check(InputComponent);
	for (const auto& NativeInput : NativeInputActions)
	{
		if (NativeInput.InputAction && (NativeInput.InputTag == InputTag))
		{
			InputHandles.Add(InputComponent->BindAction(NativeInput.InputAction, TriggerEvent, Object, Func).GetHandle());
			return;
		}
	}

	if (bLogNotFound)
	{
		DJ_LOG(DJLog, Error, TEXT("Can't find NativeInputAction for InputTag [%s] on NativeInputActions."), *InputTag.ToString());
	}
}