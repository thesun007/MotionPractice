// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "DJAT_TickBind.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTickTaskDelegate, float, DeltaTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTaskDelegate);
/**
 * 
 */
UCLASS()
class DJGAME_API UDJAT_TickBind : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	UDJAT_TickBind(const FObjectInitializer& ObjectInitializer);

	//~ UGameplayTask ������
	virtual void TickTask(float DeltaTime) override;	// ticking Target Actor ����

	UFUNCTION(BlueprintCallable, Category = "Ability Tasks", Meta = (DisplayName = "TriggerTask",
	HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "True"))
		static UDJAT_TickBind* CreateTickBindTask(UGameplayAbility* OwningAbility, FName TaskInstanceName);

private:
	//~ UAbilityTask ������
	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;		//EndTask()�� ����Ǹ� ȣ��� ��.
	virtual void ExternalCancel() override;
	//~ UAbilityTask ��

public:
	FTickTaskDelegate OnTick;
	FTaskDelegate OnCancel;

private:
	bool bActivate;
};
