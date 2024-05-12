// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "DJAT_Trigger.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTriggerDataDelegate, const FGameplayAbilityTargetDataHandle&, Data);

/**
 * Ticking 기능 추가 트리거 태스크.
 * 타겟 액터를 생성해 부딪친 액터를 반환함.
 * Confirm 방식이라면 Tick Enable 무시.
 */
UCLASS()
class DJGAME_API UDJAT_Trigger : public UAbilityTask
{
public:
	GENERATED_BODY()
	
	UDJAT_Trigger(/*const FObjectInitializer& ObjectInitializer*/);

	//~ UGameplayTask 재정의
	virtual void TickTask(float DeltaTime) override;	// ticking Target Actor 실행
	//~

	//~ UAbilityTask 함수 재정의 및 필수 함수
	UFUNCTION(BlueprintCallable, Category = "Ability Tasks", Meta = (
	HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "True"))
		static UDJAT_Trigger* CreateTriggerTask(UGameplayAbility* OwningAbility, FName TaskInstanceName, TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType, bool EnableTick,
			TSubclassOf<AGameplayAbilityTargetActor> Class);

	UFUNCTION(BlueprintCallable, Category = "Ability Tasks", Meta = (
	HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "True"))
		static UDJAT_Trigger* CreateTriggerTaskUsingActor(UGameplayAbility* OwningAbility, FName TaskInstanceName, TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType, bool EnableTick,
			AGameplayAbilityTargetActor* TargetActor);

	// "CreateTriggerTask()" 했을 때, Activate() 대신 실행
	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"), Category = "Ability Tasks")
	bool BeginSpawningActor(UGameplayAbility* OwningAbility, TSubclassOf< class AGameplayAbilityTargetActor> Class, class AGameplayAbilityTargetActor*& SpawnedActor);

	// BeginSpawningActor() 이어서 호출됨.
	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"), Category = "Ability Tasks")
	void FinishSpawningActor(UGameplayAbility* OwningAbility, class AGameplayAbilityTargetActor* SpawnedActor);															

	/** Called when the ability is asked to confirm from an outside node. What this means depends on the individual task. By default, this does nothing other than ending if bEndTask is true. */
	//virtual void ExternalConfirm(bool bEndTask) override;

	/** Called when the ability is asked to cancel from an outside node. What this means depends on the individual task. By default, this does nothing other than ending the task. */
	//virtual void ExternalCancel() override;
	//~ UAbilityTask 끝

	void SetTick(bool bEnable) { bTickingTask = bEnable; }

private:

	//~ UAbilityTask 재정의
	virtual void Activate() override;					//"CreateTriggerTaskUsingActor()" 했을 때, 실행 됨.
	virtual void OnDestroy(bool AbilityEnded) override;	//EndTask()가 실행되면 호출될 것.
	//~ UAbilityTask 끝

	virtual bool ShouldSpawnTargetActor() const;		//타겟 액터를 만들 조건이 되는지? (로컬 컨트롤, 리플리케이트 확인)
	virtual void InitializeTargetActor(AGameplayAbilityTargetActor* SpawnedActor) const;	// 타겟 액터 지연 생성, 초기화, 델리게이트 바인딩
	virtual void FinalizeTargetActor(AGameplayAbilityTargetActor* SpawnedActor) const;		// 타겟 액터 관련 나머지 설정 및 실행
	
	virtual void RegisterTargetDataCallbacks();				// 현재 서버인데, 로컬 컨트롤이 아닌 경우 클라로부터 리플리케이션 받는 준비
	virtual bool ShouldReplicateDataToServer() const;		// 서버에 리플리케이트 해야되는지? (actorInfo로 Authority 확인, TargetActor로 should~onServer 속성 확인)

	UFUNCTION()
	virtual void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& Data);		// 타겟 액터로부터 컨펌 콜백 함수

	UFUNCTION()
	virtual void OnTargetDataCancelledCallback(const FGameplayAbilityTargetDataHandle& Data);	// 타겟 액터로부터 취소 콜백 함수

	UFUNCTION()
	virtual void OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ActivationTag);		//(서버용) 클라이언트가 보낸 타겟 데이터 바인드용 콜백함수

	UFUNCTION()
	virtual void OnTargetDataReplicatedCancelledCallback();		//(서버용) 클라이언트가 보낸 타겟 액터 취소 바인드용 콜백함수

public:
	UPROPERTY(BlueprintAssignable)
	FTriggerDataDelegate	ValidData;

	UPROPERTY(BlueprintAssignable)
	FTriggerDataDelegate	Canceled;
	UPROPERTY()
	TSubclassOf<AGameplayAbilityTargetActor> TargetClass;

	/** The TargetActor that we spawned */
	UPROPERTY()
	TObjectPtr<AGameplayAbilityTargetActor> TargetActor;

	TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType;
};
