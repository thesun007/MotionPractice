// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GA/DJGameplayAbility.h"
#include "DJGA_Sliding.generated.h"

/**
 * 
 */
UCLASS()
class DJGAME_API UDJGA_Sliding : public UDJGameplayAbility
{
	GENERATED_BODY()
	
public:
	UDJGA_Sliding(const FObjectInitializer& ObjectInitializer);

protected:
	//~ GameAbility �Լ� ������
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	//~ ��

private:
	//(��Ÿ�� ���н�)(�½�ũ �����) ���� �Լ�
	UFUNCTION()
	void EndFunction();
	//������ ��� ���� �Լ�
	UFUNCTION()
	void StartEnding();
	//��Ÿ�� ����� �ݹ� �Լ�
	UFUNCTION()
	void CheckUpper();
	//���� ��ֹ� Ȯ�� �½�ũ �ݹ��Լ�
	UFUNCTION()
	void OnGetTargetCallback(const struct FGameplayAbilityTargetDataHandle& TargetDataHandle);

	UFUNCTION()
	void Tick_ProcessOnGround(float DeltaTime);
	void ProcessMontageNTask(FVector Dir, float Strength);
	//������ ���� ���� �˸�
	UFUNCTION()
	void MontageBeginNotifyCallback(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAnimMontage> SlidingMontage;
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	float Duration;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float MaxFloorAngle;	//Maximum sliding floor angle
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float Deceleration;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float FinalDeceleration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AssassinationSetting", meta = (AllowPrivateAccess = "true"))
	uint8 bDebug : 1 = false;

	UPROPERTY()
	TObjectPtr<class ADJCharacterBase> Character;
	UPROPERTY()
	TObjectPtr<class UCharacterMovementComponent> Movement;

	UPROPERTY()
	TObjectPtr<class ATargetActor_TickableRadius> TargetActorRadius;	// ��ֹ�, �Ұ��� ���� ���� ���ÿ�
	float OriginalDeceleration;

	FTimerHandle RotateTimerHandle;
	bool bEndCommand = false;
	bool IsEnding= false;	//������ ����(Sliding End) ����������?
};
