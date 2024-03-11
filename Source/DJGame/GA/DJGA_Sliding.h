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
	//~ GameAbility 함수 재정의
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	//~ 끝

private:
	//(몽타주 실패시)(태스크 종료시) 종료 함수
	UFUNCTION()
	void EndFunction();
	//마무리 모션 진행 함수
	UFUNCTION()
	void StartEnding();
	//몽타주 종료시 콜백 함수
	UFUNCTION()
	void CheckUpper();
	//정면 장애물 확인 태스크 콜백함수
	UFUNCTION()
	void OnGetTargetCallback(const struct FGameplayAbilityTargetDataHandle& TargetDataHandle);

	UFUNCTION()
	void Tick_ProcessOnGround(float DeltaTime);
	void ProcessMontageNTask(FVector Dir, float Strength);
	//마무리 동작 시작 알림
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
	TObjectPtr<class ATargetActor_TickableRadius> TargetActorRadius;	// 장애물, 불가능 지면 각도 감시용
	float OriginalDeceleration;

	FTimerHandle RotateTimerHandle;
	bool bEndCommand = false;
	bool IsEnding= false;	//마무리 동작(Sliding End) 실행중인지?
};
