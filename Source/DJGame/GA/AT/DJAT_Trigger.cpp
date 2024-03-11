// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/AT/DJAT_Trigger.h"
#include "AbilitySystemComponent.h"
//#include "EngineGlobals.h"
//#include "Engine/Engine.h"
UDJAT_Trigger::UDJAT_Trigger()
{
	bTickingTask = false;
}

void UDJAT_Trigger::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (TargetActor->ShouldProduceTargetData())
	{
		// If instant confirm, then stop targeting immediately.
		// Note this is kind of bad: we should be able to just call a static func on the CDO to do this. 
		// But then we wouldn't get to set ExposeOnSpawnParameters.
		if (ConfirmationType == EGameplayTargetingConfirmation::Instant)
		{
			TargetActor->ConfirmTargeting();
		}
	}
}

UDJAT_Trigger* UDJAT_Trigger::CreateTriggerTask(UGameplayAbility* OwningAbility, FName TaskInstanceName, TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType, bool EnableTick, TSubclassOf<AGameplayAbilityTargetActor> Class)
{
	UDJAT_Trigger* MyObj = NewAbilityTask<UDJAT_Trigger>(OwningAbility, TaskInstanceName);		//Register for task list here, providing a given FName as a key
	MyObj->TargetClass = Class;
	MyObj->TargetActor = nullptr;
	MyObj->ConfirmationType = ConfirmationType;
	MyObj->bTickingTask = EnableTick;
	return MyObj;
}

UDJAT_Trigger* UDJAT_Trigger::CreateTriggerTaskUsingActor(UGameplayAbility* OwningAbility, FName TaskInstanceName, TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType, bool EnableTick, AGameplayAbilityTargetActor* TargetActor)
{
	UDJAT_Trigger* MyObj = NewAbilityTask<UDJAT_Trigger>(OwningAbility, TaskInstanceName);		//Register for task list here, providing a given FName as a key
	MyObj->TargetClass = nullptr;
	MyObj->TargetActor = TargetActor;
	MyObj->ConfirmationType = ConfirmationType;
	MyObj->bTickingTask = EnableTick;
	return MyObj;
}

bool UDJAT_Trigger::BeginSpawningActor(UGameplayAbility* OwningAbility, TSubclassOf<class AGameplayAbilityTargetActor> Class, AGameplayAbilityTargetActor*& SpawnedActor)
{
	SpawnedActor = nullptr;

	if (Ability)
	{
		if (ShouldSpawnTargetActor())
		{
			UClass* _Class = *Class;
			if (_Class != nullptr)
			{
				if (UWorld* World = GEngine->GetWorldFromContextObject(OwningAbility, EGetWorldErrorMode::LogAndReturnNull))
				{
					SpawnedActor = World->SpawnActorDeferred<AGameplayAbilityTargetActor>(_Class, FTransform::Identity, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
				}
			}

			if (SpawnedActor)
			{
				TargetActor = SpawnedActor;
				InitializeTargetActor(TargetActor);
			}
		}

		RegisterTargetDataCallbacks();
	}

	return (SpawnedActor != nullptr);
}

void UDJAT_Trigger::FinishSpawningActor(UGameplayAbility* OwningAbility, AGameplayAbilityTargetActor* SpawnedActor)
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (ASC && IsValid(SpawnedActor))
	{
		check(TargetActor == SpawnedActor);

		const FTransform SpawnTransform = ASC->GetOwner()->GetTransform();

		SpawnedActor->FinishSpawning(SpawnTransform);

		FinalizeTargetActor(SpawnedActor);
	}
}

void UDJAT_Trigger::Activate()
{
	// Need to handle case where target actor was passed into task
	if (Ability && (TargetClass == nullptr))
	{
		if (TargetActor)
		{
			AGameplayAbilityTargetActor* SpawnedActor = TargetActor;
			TargetClass = SpawnedActor->GetClass();

			RegisterTargetDataCallbacks();


			if (!IsValid(this))
			{
				return;
			}

			if (ShouldSpawnTargetActor())
			{
				InitializeTargetActor(SpawnedActor);
				FinalizeTargetActor(SpawnedActor);

				// Note that the call to FinalizeTargetActor, this task could finish and our owning ability may be ended.
			}
			else
			{
				TargetActor = nullptr;

				// We may need a better solution here.  We don't know the target actor isn't needed till after it's already been spawned.
				SpawnedActor->Destroy();
				SpawnedActor = nullptr;
			}
		}
		else
		{
			EndTask();
		}
	}
}

void UDJAT_Trigger::OnDestroy(bool AbilityEnded)
{
	if (TargetActor)
	{
		TargetActor->Destroy();
	}

	Super::OnDestroy(AbilityEnded);
}

bool UDJAT_Trigger::ShouldSpawnTargetActor() const
{
	check(TargetClass);
	check(Ability);

	//���� ��Ʈ�� �̰ų� ���ø�����Ʈ Ȱ��ȭ �Ǵ� �������� �����Ǵ°Ÿ� true.
	// (E.g., ������ �� ��� ���͸� �����Ͽ� �������� ���� ��� Ŭ���̾�Ʈ�� �����մϴ�)

	const AGameplayAbilityTargetActor* CDO = CastChecked<AGameplayAbilityTargetActor>(TargetClass->GetDefaultObject());

	const bool bReplicates = CDO->GetIsReplicated();
	const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
	const bool bShouldProduceTargetDataOnServer = CDO->ShouldProduceTargetDataOnServer;		//"�������� ������ ����" �ɼ��� �����ִ���?

	return (bReplicates || bIsLocallyControlled || bShouldProduceTargetDataOnServer);
}

void UDJAT_Trigger::InitializeTargetActor(AGameplayAbilityTargetActor* SpawnedActor) const
{
	//Ÿ�� ���� �������� ��, �ʱ�ȭ �۾� ����
	check(SpawnedActor);
	check(Ability);

	SpawnedActor->PrimaryPC = Ability->GetCurrentActorInfo()->PlayerController.Get();

	// If we spawned the target actor, always register the callbacks for when the data is ready.
	SpawnedActor->TargetDataReadyDelegate.AddUObject(const_cast<UDJAT_Trigger*>(this), &UDJAT_Trigger::OnTargetDataReadyCallback);
	SpawnedActor->CanceledDelegate.AddUObject(const_cast<UDJAT_Trigger*>(this), &UDJAT_Trigger::OnTargetDataCancelledCallback);
}

void UDJAT_Trigger::FinalizeTargetActor(AGameplayAbilityTargetActor* SpawnedActor) const
{
	check(SpawnedActor);
	check(Ability);

	if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
	{
		// User ability activation is inhibited while this is active
		ASC->SpawnedTargetActors.Push(SpawnedActor);	//ASC�� �������� Ÿ�� ���͸� confirm �� �� ����.
	}

	SpawnedActor->StartTargeting(Ability);

	if (TargetActor->ShouldProduceTargetData())	//���� ��Ʈ�ѷ��̰ų� ShouldProduceTargetDataOnServer = true �̸�
	{
		// If instant confirm, then stop targeting immediately.
		// Note this is kind of bad: we should be able to just call a static func on the CDO to do this. 
		// But then we wouldn't get to set ExposeOnSpawnParameters.
		if (bTickingTask == false && ConfirmationType == EGameplayTargetingConfirmation::Instant)		// �ν��Ͻ� ����̸� Ticking�� �ƴҶ��� �ٷ� ����
		{
			TargetActor->ConfirmTargeting();
		}
		else if (ConfirmationType == EGameplayTargetingConfirmation::UserConfirmed)
		{
			// Bind to the Cancel/Confirm Delegates (called from local confirm or from repped confirm), ���� ����̸� Ticking ������� �ѹ��� ���ε�
			TargetActor->BindToConfirmCancelInputs();
		}
	}
}

void UDJAT_Trigger::RegisterTargetDataCallbacks()
{
	if (!ensure(IsValid(this)))
	{
		return;
	}

	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!ASC)
	{
		return;
	}

	check(TargetClass);
	check(Ability);

	const AGameplayAbilityTargetActor* TA_CDO = CastChecked<AGameplayAbilityTargetActor>(TargetClass->GetDefaultObject());

	const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
	const bool bShouldProduceTargetDataOnServer = TA_CDO->ShouldProduceTargetDataOnServer;


	if (!bIsLocallyControlled)	//���� ���� ��Ʈ���� �ƴѵ�,
	{
		// Ÿ�� ���Ͱ� Ŭ���̾�Ʈ���� ����Ǵ� �Ÿ�
		// Ŭ���̾�Ʈ�� ������ ������ ����Ǵ� ��� TargetData �ݹ鿡 ���.
		if (!bShouldProduceTargetDataOnServer)
		{
			FGameplayAbilitySpecHandle	SpecHandle = GetAbilitySpecHandle();
			FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();

			//Since multifire is supported, we still need to hook up the callbacks
			//Ŭ���̾�Ʈ�� ���� Target Actor Data�� ���� �ݹ� �Լ� ����
			ASC->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UDJAT_Trigger::OnTargetDataReplicatedCallback);
			ASC->AbilityTargetDataCancelledDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UDJAT_Trigger::OnTargetDataReplicatedCancelledCallback);

			ASC->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);	//�� �����Ƽ�� ���� Ÿ�� ������ ���ø�����Ʈ�� �̹� �ִٸ� �ٷ� ���ø�����Ʈ �ݹ� �Լ� ȣ��

			SetWaitingOnRemotePlayerData();
		}
	}
}

bool UDJAT_Trigger::ShouldReplicateDataToServer() const
{
	if (!Ability || !TargetActor)
	{
		return false;
	}

	// TargetData�� ������ �����ϴ�. IFF ���� Ŭ���̾�Ʈ�̰�, �̰��� �������� �����͸� ������ �� �ִ� GameplayTargetActor�� �ƴմϴ�.
	const FGameplayAbilityActorInfo* Info = Ability->GetCurrentActorInfo();
	if (!Info->IsNetAuthority() && !TargetActor->ShouldProduceTargetDataOnServer)
	{
		return true;
	}

	return false;
}

void UDJAT_Trigger::OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& Data)
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!Ability || !ASC)
	{
		return;
	}

	//�ǹ� �Ҹ�, ���� Ű ����?
	FScopedPredictionWindow	ScopedPrediction(ASC, ShouldReplicateDataToServer()); // [Ŭ���̰� ShouldProduceTargetDataOnServer=false] �� true

	const FGameplayAbilityActorInfo* Info = Ability->GetCurrentActorInfo();
	if (IsPredictingClient())	// ���� �����Ǵ� ���� ��Ʈ�� Ŭ���̾�Ʈ�� ��Ȳ��
	{
		if (!TargetActor->ShouldProduceTargetDataOnServer)	// Ŭ�󿡼� ����Ǵ� Ÿ�� ���͸�
		{
			FGameplayTag ApplicationTag; // Fixme: where would this be useful?opedPredictionKey);
	
			//������ Ÿ�� ������ ���ø�����Ʈ ( AbilityTargetDataSetDelegate() �� ����)
			ASC->CallServerSetReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey(), Data, ApplicationTag, ASC->ScopedPredictionKey);
		}
		else if (ConfirmationType == EGameplayTargetingConfirmation::UserConfirmed)	//�������� ����Ǵ� Ÿ�� �����̸�, UserConfirm ����̸�
		{
			// ��� �����͸� ������ ���� �������� �Ϲ����� ���� �޽����� ���� ��.(RPC) (�Ƹ� �������� ���� Ÿ�� �����͸� ���� ����) (AbilityReplicatedEventDelegate() �� �����, BindToConfirmCancelInputs() Ȯ�� )
			ASC->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericConfirm, GetAbilitySpecHandle(), GetActivationPredictionKey(), ASC->ScopedPredictionKey);
		}
	}

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(Data);
	}

	//if (ConfirmationType != EGameplayTargetingConfirmation::CustomMulti)
	if(bTickingTask == false && TargetActor->IsActorTickEnabled()== false)
	{
		EndTask();
	}
}

void UDJAT_Trigger::OnTargetDataCancelledCallback(const FGameplayAbilityTargetDataHandle& Data)
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!ASC)
	{
		return;
	}

	FScopedPredictionWindow ScopedPrediction(ASC, IsPredictingClient());

	if (IsPredictingClient())
	{
		if (!TargetActor->ShouldProduceTargetDataOnServer)
		{
			ASC->ServerSetReplicatedTargetDataCancelled(GetAbilitySpecHandle(), GetActivationPredictionKey(), ASC->ScopedPredictionKey);	//������ ��� ����.
		}
		else
		{
			// ��� �����͸� ������ ���� �������� �Ϲ����� Ȯ�� �޽����� ���� ��.
			ASC->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericCancel, GetAbilitySpecHandle(), GetActivationPredictionKey(), ASC->ScopedPredictionKey);
		}
	}

	if(ShouldBroadcastAbilityTaskDelegates())
		Canceled.Broadcast(Data);

	EndTask();
}

//Ŭ���̾�Ʈ�� �����͸� ������ ȣ�� ��.
void UDJAT_Trigger::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ActivationTag)	
{
	FGameplayAbilityTargetDataHandle MutableData = Data;

	if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
	{
		ASC->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());	//�ش� Ÿ�� �����͸� �Һ�(����)
	}

	/**
	 *  TargetActor�� ȣ���Ͽ� �����͸� ����/�����մϴ�. �̰��� false�� ��ȯ�ϸ� ������ ��� �����͸� �ź��ϰ� �̸� ��ҷ� ó���մϴ�.
	 *	�̴� �뿪�� ����ȭ���� ���� �� �ֽ��ϴ�. OnReplicatedTargetDataReceived()�� ���� ������ ���� ����/Ȯ��/��� �۾��� �����ϰ� �ش� �����͸� ����� �� �ֽ��ϴ�.
	 *	���� Ŭ���̾�Ʈ�� �ش� �����͸� ��������� �������� �ϴ� ��� Ŭ���̾�Ʈ�� �⺻������ 'Ȯ��'�� ������ ������ ���� OnReplicatedTargetDataReceived���� �۾��� �����ϰ� �˴ϴ�.
	 */
	if (TargetActor && !TargetActor->OnReplicatedTargetDataReceived(MutableData))	//Ÿ�� �����Ͱ� ��ȿ���� ������, cancel ��ε� ĳ��Ʈ
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			Canceled.Broadcast(MutableData);
		}
	}
	else
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			ValidData.Broadcast(MutableData);
		}
	}

	//if (ConfirmationType != EGameplayTargetingConfirmation::CustomMulti)
	if (bTickingTask == false)
	{
		EndTask();
	}
}

//Ŭ���̾�Ʈ�� �����͸� ������ ȣ�� ��.
void UDJAT_Trigger::OnTargetDataReplicatedCancelledCallback()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		Canceled.Broadcast(FGameplayAbilityTargetDataHandle());
	}
	EndTask();
}
