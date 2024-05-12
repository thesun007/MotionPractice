// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "DJAT_Trigger.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTriggerDataDelegate, const FGameplayAbilityTargetDataHandle&, Data);

/**
 * Ticking ��� �߰� Ʈ���� �½�ũ.
 * Ÿ�� ���͸� ������ �ε�ģ ���͸� ��ȯ��.
 * Confirm ����̶�� Tick Enable ����.
 */
UCLASS()
class DJGAME_API UDJAT_Trigger : public UAbilityTask
{
public:
	GENERATED_BODY()
	
	UDJAT_Trigger(/*const FObjectInitializer& ObjectInitializer*/);

	//~ UGameplayTask ������
	virtual void TickTask(float DeltaTime) override;	// ticking Target Actor ����
	//~

	//~ UAbilityTask �Լ� ������ �� �ʼ� �Լ�
	UFUNCTION(BlueprintCallable, Category = "Ability Tasks", Meta = (
	HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "True"))
		static UDJAT_Trigger* CreateTriggerTask(UGameplayAbility* OwningAbility, FName TaskInstanceName, TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType, bool EnableTick,
			TSubclassOf<AGameplayAbilityTargetActor> Class);

	UFUNCTION(BlueprintCallable, Category = "Ability Tasks", Meta = (
	HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "True"))
		static UDJAT_Trigger* CreateTriggerTaskUsingActor(UGameplayAbility* OwningAbility, FName TaskInstanceName, TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType, bool EnableTick,
			AGameplayAbilityTargetActor* TargetActor);

	// "CreateTriggerTask()" ���� ��, Activate() ��� ����
	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"), Category = "Ability Tasks")
	bool BeginSpawningActor(UGameplayAbility* OwningAbility, TSubclassOf< class AGameplayAbilityTargetActor> Class, class AGameplayAbilityTargetActor*& SpawnedActor);

	// BeginSpawningActor() �̾ ȣ���.
	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"), Category = "Ability Tasks")
	void FinishSpawningActor(UGameplayAbility* OwningAbility, class AGameplayAbilityTargetActor* SpawnedActor);															

	/** Called when the ability is asked to confirm from an outside node. What this means depends on the individual task. By default, this does nothing other than ending if bEndTask is true. */
	//virtual void ExternalConfirm(bool bEndTask) override;

	/** Called when the ability is asked to cancel from an outside node. What this means depends on the individual task. By default, this does nothing other than ending the task. */
	//virtual void ExternalCancel() override;
	//~ UAbilityTask ��

	void SetTick(bool bEnable) { bTickingTask = bEnable; }

private:

	//~ UAbilityTask ������
	virtual void Activate() override;					//"CreateTriggerTaskUsingActor()" ���� ��, ���� ��.
	virtual void OnDestroy(bool AbilityEnded) override;	//EndTask()�� ����Ǹ� ȣ��� ��.
	//~ UAbilityTask ��

	virtual bool ShouldSpawnTargetActor() const;		//Ÿ�� ���͸� ���� ������ �Ǵ���? (���� ��Ʈ��, ���ø�����Ʈ Ȯ��)
	virtual void InitializeTargetActor(AGameplayAbilityTargetActor* SpawnedActor) const;	// Ÿ�� ���� ���� ����, �ʱ�ȭ, ��������Ʈ ���ε�
	virtual void FinalizeTargetActor(AGameplayAbilityTargetActor* SpawnedActor) const;		// Ÿ�� ���� ���� ������ ���� �� ����
	
	virtual void RegisterTargetDataCallbacks();				// ���� �����ε�, ���� ��Ʈ���� �ƴ� ��� Ŭ��κ��� ���ø����̼� �޴� �غ�
	virtual bool ShouldReplicateDataToServer() const;		// ������ ���ø�����Ʈ �ؾߵǴ���? (actorInfo�� Authority Ȯ��, TargetActor�� should~onServer �Ӽ� Ȯ��)

	UFUNCTION()
	virtual void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& Data);		// Ÿ�� ���ͷκ��� ���� �ݹ� �Լ�

	UFUNCTION()
	virtual void OnTargetDataCancelledCallback(const FGameplayAbilityTargetDataHandle& Data);	// Ÿ�� ���ͷκ��� ��� �ݹ� �Լ�

	UFUNCTION()
	virtual void OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ActivationTag);		//(������) Ŭ���̾�Ʈ�� ���� Ÿ�� ������ ���ε�� �ݹ��Լ�

	UFUNCTION()
	virtual void OnTargetDataReplicatedCancelledCallback();		//(������) Ŭ���̾�Ʈ�� ���� Ÿ�� ���� ��� ���ε�� �ݹ��Լ�

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
