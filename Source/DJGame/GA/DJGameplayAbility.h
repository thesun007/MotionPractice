// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "DJGameplayAbility.generated.h"


UENUM(BlueprintType)
enum class EDJAbilityActivationPolicy : uint8
{
	// Try to activate the ability when the input is triggered. (��ǲ ��, ó���� ActivateAbility(). ����ִٸ� �������� InputPressed())
	OnInputTriggered,

	// Continually try to activate the ability while the input is active. (��ǲ ���Ŀ�, ���������� ActivateAbility() �����.)
	WhileInputActive,

	// Try to activate the ability when an avatar is assigned. (���� ��, �̸� ActivateAbility() ����, ���� ��� �ִٸ� �Է½� InputPressed())
	OnSpawn
};


/**
 * 
 */
UCLASS(Abstract, HideCategories = Input, Meta = (ShortTooltip = "The base gameplay ability class used by this project."))
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

	//���� Ȥ�� OnGiveAbility() �Ҷ� �õ�
	void TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const;

protected:
	//~UGameplayAbility interface
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	//~End

	/** Called when this ability is granted to the ability system component. */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityAdded")
	void K2_OnAbilityAdded();

protected:
	// Defines how this ability is meant to activate.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Ability Activation")
	EDJAbilityActivationPolicy ActivationPolicy;
};
