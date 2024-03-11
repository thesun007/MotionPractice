// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GA/DJGameplayAbility.h"
#include "NativeGameplayTags.h"
#include "DJGA_AssassinationVictim.generated.h"


UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_InputTag_AssassinationVictim);

/**
 * 
 */
UCLASS()
class DJGAME_API UDJGA_AssassinationVictim : public UDJGameplayAbility
{
	GENERATED_BODY()
	
public:
	UDJGA_AssassinationVictim(const FObjectInitializer& ObjectInitializer);

protected:
	//~ GameAbility �Լ� ������
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	//virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	//~ ��

	//(��Ÿ�� ���н�)(�½�ũ �����) ���� �Լ�
	UFUNCTION()
	void EndFunction();
	UFUNCTION()
	void MontageBeginNotifyCallback(FName NotifyName, const struct FBranchingPointNotifyPayload& BranchingPointPayload);

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AssassinationAnimation", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<class UAnimMontage>> AssassinationVictimMontages;

	ADJCharacterBase* Character = nullptr;
};
