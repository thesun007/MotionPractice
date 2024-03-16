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

	//~ UGameplayTask 재정의
	virtual void TickTask(float DeltaTime) override;	// ticking Target Actor 실행

	UFUNCTION(BlueprintCallable, Category = "Ability Tasks", Meta = (DisplayName = "TriggerTask",
	HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "True"))
		static UDJAT_TickBind* CreateTickBindTask(UGameplayAbility* OwningAbility, FName TaskInstanceName);

private:
	//~ UAbilityTask 재정의
	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;		//EndTask()가 실행되면 호출될 것.
	virtual void ExternalCancel() override;
	//~ UAbilityTask 끝

public:
	FTickTaskDelegate OnTick;
	FTaskDelegate OnCancel;

private:
	bool bActivate;
};
