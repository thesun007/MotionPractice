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

	//로컬 컨트롤 이거나 리플리케이트 활성화 또는 서버에서 생성되는거면 true.
	// (E.g., 서버는 이 대상 액터를 생성하여 소유하지 않은 모든 클라이언트에 복제합니다)

	const AGameplayAbilityTargetActor* CDO = CastChecked<AGameplayAbilityTargetActor>(TargetClass->GetDefaultObject());

	const bool bReplicates = CDO->GetIsReplicated();
	const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
	const bool bShouldProduceTargetDataOnServer = CDO->ShouldProduceTargetDataOnServer;		//"서버에서 무조건 생성" 옵션이 켜져있는지?

	return (bReplicates || bIsLocallyControlled || bShouldProduceTargetDataOnServer);
}

void UDJAT_Trigger::InitializeTargetActor(AGameplayAbilityTargetActor* SpawnedActor) const
{
	//타겟 액터 지연생성 후, 초기화 작업 진행
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
		ASC->SpawnedTargetActors.Push(SpawnedActor);	//ASC가 수동으로 타겟 액터를 confirm 할 수 있음.
	}

	SpawnedActor->StartTargeting(Ability);

	if (TargetActor->ShouldProduceTargetData())	//로컬 컨트롤러이거나 ShouldProduceTargetDataOnServer = true 이면
	{
		// If instant confirm, then stop targeting immediately.
		// Note this is kind of bad: we should be able to just call a static func on the CDO to do this. 
		// But then we wouldn't get to set ExposeOnSpawnParameters.
		if (bTickingTask == false && ConfirmationType == EGameplayTargetingConfirmation::Instant)		// 인스턴스 방식이면 Ticking이 아닐때만 바로 실행
		{
			TargetActor->ConfirmTargeting();
		}
		else if (ConfirmationType == EGameplayTargetingConfirmation::UserConfirmed)
		{
			// Bind to the Cancel/Confirm Delegates (called from local confirm or from repped confirm), 컴펌 방식이면 Ticking 상관없이 한번만 바인딩
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


	if (!bIsLocallyControlled)	//현재 로컬 컨트롤이 아닌데,
	{
		// 타겟 액터가 클라이언트에서 실행되는 거면
		// 클라이언트가 전송할 것으로 예상되는 경우 TargetData 콜백에 등록.
		if (!bShouldProduceTargetDataOnServer)
		{
			FGameplayAbilitySpecHandle	SpecHandle = GetAbilitySpecHandle();
			FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();

			//Since multifire is supported, we still need to hook up the callbacks
			//클라이언트가 보낸 Target Actor Data에 대한 콜백 함수 지정
			ASC->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UDJAT_Trigger::OnTargetDataReplicatedCallback);
			ASC->AbilityTargetDataCancelledDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UDJAT_Trigger::OnTargetDataReplicatedCancelledCallback);

			ASC->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);	//이 어빌리티에 대한 타겟 데이터 리플리케이트가 이미 있다면 바로 리플리케이트 콜백 함수 호출

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

	// TargetData를 서버로 보냅니다. IFF 현재 클라이언트이고, 이것은 서버에서 데이터를 생성할 수 있는 GameplayTargetActor가 아닙니다.
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

	//의미 불명, 예측 키 생성?
	FScopedPredictionWindow	ScopedPrediction(ASC, ShouldReplicateDataToServer()); // [클라이고 ShouldProduceTargetDataOnServer=false] 면 true

	const FGameplayAbilityActorInfo* Info = Ability->GetCurrentActorInfo();
	if (IsPredictingClient())	// 현재 예측되는 로컬 컨트롤 클라이언트인 상황에
	{
		if (!TargetActor->ShouldProduceTargetDataOnServer)	// 클라에서 실행되는 타겟 액터면
		{
			FGameplayTag ApplicationTag; // Fixme: where would this be useful?opedPredictionKey);
	
			//서버로 타켓 데이터 리플리케이트 ( AbilityTargetDataSetDelegate() 와 연계)
			ASC->CallServerSetReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey(), Data, ApplicationTag, ASC->ScopedPredictionKey);
		}
		else if (ConfirmationType == EGameplayTargetingConfirmation::UserConfirmed)	//서버에서 실행되는 타겟 액터이며, UserConfirm 모드이면
		{
			// 대상 데이터를 보내지 않을 것이지만 일반적인 컨펌 메시지를 보낼 것.(RPC) (아마 서버에서 직접 타겟 데이터를 얻을 것임) (AbilityReplicatedEventDelegate() 와 연계됨, BindToConfirmCancelInputs() 확인 )
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
			ASC->ServerSetReplicatedTargetDataCancelled(GetAbilitySpecHandle(), GetActivationPredictionKey(), ASC->ScopedPredictionKey);	//서버에 취소 전송.
		}
		else
		{
			// 대상 데이터를 보내지 않을 것이지만 일반적인 확인 메시지를 보낼 것.
			ASC->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericCancel, GetAbilitySpecHandle(), GetActivationPredictionKey(), ASC->ScopedPredictionKey);
		}
	}

	if(ShouldBroadcastAbilityTaskDelegates())
		Canceled.Broadcast(Data);

	EndTask();
}

//클라이언트가 데이터를 보내서 호출 됨.
void UDJAT_Trigger::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ActivationTag)	
{
	FGameplayAbilityTargetDataHandle MutableData = Data;

	if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
	{
		ASC->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());	//해당 타겟 데이터를 소비(지움)
	}

	/**
	 *  TargetActor를 호출하여 데이터를 정리/검증합니다. 이것이 false를 반환하면 복제된 대상 데이터를 거부하고 이를 취소로 처리합니다.
	 *	이는 대역폭 최적화에도 사용될 수 있습니다. OnReplicatedTargetDataReceived()는 서버 측에서 실제 추적/확인/모든 작업을 수행하고 해당 데이터를 사용할 수 있습니다.
	 *	따라서 클라이언트가 해당 데이터를 명시적으로 보내도록 하는 대신 클라이언트는 기본적으로 '확인'만 보내고 서버는 이제 OnReplicatedTargetDataReceived에서 작업을 수행하게 됩니다.
	 */
	if (TargetActor && !TargetActor->OnReplicatedTargetDataReceived(MutableData))	//타겟 데이터가 유효하지 않으면, cancel 브로드 캐스트
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

//클라이언트가 데이터를 보내서 호출 됨.
void UDJAT_Trigger::OnTargetDataReplicatedCancelledCallback()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		Canceled.Broadcast(FGameplayAbilityTargetDataHandle());
	}
	EndTask();
}
