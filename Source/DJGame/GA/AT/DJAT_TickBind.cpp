// Fill out your copyright notice in the Description page of Project Settings.


#include "DJAT_TickBind.h"

UDJAT_TickBind::UDJAT_TickBind(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	bTickingTask = true;
	bActivate = false;
}

void UDJAT_TickBind::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (bActivate)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnTick.Broadcast(DeltaTime);
		}
	}
}

UDJAT_TickBind* UDJAT_TickBind::CreateTickBindTask(UGameplayAbility* OwningAbility, FName TaskInstanceName)
{
	UDJAT_TickBind* Instance = NewAbilityTask<UDJAT_TickBind>(OwningAbility, TaskInstanceName);
	return Instance;
}

void UDJAT_TickBind::Activate()
{
	bActivate = true;
}

void UDJAT_TickBind::OnDestroy(bool bInOwnerFinished)
{
	Super::OnDestroy(bInOwnerFinished);
}

void UDJAT_TickBind::ExternalCancel()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnCancel.Broadcast();
	}
	Super::ExternalCancel();
}
