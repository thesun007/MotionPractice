// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/DJGA_Assassination.h"
#include "AT/DJAT_Trigger.h"
#include "Abilities/GameplayAbilityTargetActor_Radius.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/DJCharacterBase.h"
#include "MotionWarpingComponent.h"
#include "GAS/DJAbilitySystemComponent.h"
#include "DJGame.h"
#include "GA/DJGA_AssassinationVictim.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_InputTag_Assassination, "InputTag.Ability.Assassination");
UE_DEFINE_GAMEPLAY_TAG(TAG_AbilityType_Action_Assassination, "Ability.Type.Action.Assassination");

UDJGA_Assassination::UDJGA_Assassination(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationPolicy = EDJAbilityActivationPolicy::OnSpawn;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;	//임시
	
	AbilityTags.AddTag(TAG_AbilityType_Action_Assassination);
	FAbilityTriggerData trigger;
	trigger.TriggerTag = TAG_InputTag_Assassination;
	AbilityTriggers.Add(trigger);

	VictimAbilityTriggerTag = TAG_InputTag_AssassinationVictim;	//희생자 어빌리티 실행 태그
}

void UDJGA_Assassination::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 초기화
	Character = GetDJCharacterFromActorInfo();

	//**암살 감시 시작**//
	TriggerRadius = GetWorld()->SpawnActor<AGameplayAbilityTargetActor_Radius>(ActorInfo->AvatarActor.Get()->GetActorLocation(), ActorInfo->AvatarActor.Get()->GetActorRotation());

	//트리거 셋팅
	TriggerRadius->Radius = Radius;
	TriggerRadius->bDebug = bDebug;
	TriggerRadius->bDestroyOnConfirmation = false;		//계속 트리거 시도하기 위해 해당 옵션 비활성화
	TriggerRadius->StartLocation = MakeTargetLocationInfoFromOwnerActor();	//오너 액터의 위치를 시작위치로 설정

	//필터 적용법
	FGameplayTargetDataFilterHandle filterHandle;
	FGameplayTargetDataFilter filter;
	filter.SelfActor = CurrentActorInfo->AvatarActor.Get();
	filter.RequiredActorClass = ADJCharacterBase::StaticClass();
	filter.SelfFilter = ETargetDataFilterSelf::TDFS_NoSelf;
	filterHandle.Filter = MakeShared<FGameplayTargetDataFilter, ESPMode::ThreadSafe>(filter);
	TriggerRadius->Filter = filterHandle;
	
	//(암살 범위 트리거)어빌리티 태스크 시작
	UDJAT_Trigger* TriggerTask = UDJAT_Trigger::CreateTriggerTaskUsingActor(this, TEXT("AssassinationTask"),EGameplayTargetingConfirmation::Instant, true, TriggerRadius);	//TargetActor는 Trigger 태스크 와 함께 소멸
	TriggerTask->ValidData.AddDynamic(this, &UDJGA_Assassination::OnGetTargetCallback);
	TriggerTask->ReadyForActivation();
	
}

void UDJGA_Assassination::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	//Super::InputPressed(Handle, ActorInfo, ActivationInfo);
	//*암살 가능 대상 있다면 암살 모션 실행*//

	if (Character && TargetActor && AssassinationMontages.Num()>0)
	{
		/* 현재 ground 상태 가정 */
		FTransform TargetTransform = TargetActor->GetActorTransform();	//타겟 트랜스폼
		FVector AssassinToTarget = Character->GetActorLocation() - TargetTransform.GetLocation();	// 타겟 기준 암살자 위치

		/* 현재 가능한 암살 타입 구하기 */
		EAssassinationType GaolAssassinationType;		// 암살타입 저장 변수
		FVector AssassinToTargetDir = AssassinToTarget;	// 타겟 기준 암살자 방향
		AssassinToTargetDir.Z = 0;

		FRotator DeltaRotation = (FRotationMatrix::MakeFromX(AssassinToTargetDir).Rotator() - TargetTransform.Rotator());	// 타겟 기준, 암살자 위치의 각도.
		DeltaRotation.Normalize();

		//UE_LOG(LogTemp, Log, TEXT("Target : %f,  TargetToAssassin : %f"), TargetTransform.Rotator().Yaw, DeltaRotation.Yaw);
		if (-50. < DeltaRotation.Yaw && DeltaRotation.Yaw < 50.)
			GaolAssassinationType = EAssassinationType::front;
		else if(DeltaRotation.Yaw < -130 || DeltaRotation.Yaw > 130)
			GaolAssassinationType = EAssassinationType::Back;
		else
			GaolAssassinationType = EAssassinationType::Side;

		/* 암살 몽타주 선택 */
		TArray< FAssassinationMontageData> ListMontages;
		for (FAssassinationMontageData data : AssassinationMontages)
		{
			if (GaolAssassinationType == data.Type)
				ListMontages.Add(data);
		}

		//몽타주 없으면 취소
		if (ListMontages.IsEmpty())
		{
			return;
		}

		//Character->SetActorEnableCollision(false);
		CurrentMontageindex = FMath::RandRange(0, ListMontages.Num() - 1);


		FAssassinationMontageData SelectedMontageData = ListMontages[CurrentMontageindex];	// 최종 암살 몽타주

		/* 몽타주 실행 시도 */
		if (CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo) == false)
		{
			return;
		}

		/* 모션와핑 적용 */
		FVector AbsolutePos = TargetTransform.TransformPosition(SelectedMontageData.SyncPosition);	// 암살자가 위치해야 될 절대값
		UMotionWarpingComponent* MWC = Character->GetMotionWarpingComponent();
		if (MWC)
		{
			MWC->AddOrUpdateWarpTargetFromLocationAndRotation(FName(TEXT("StartPos")), AbsolutePos, TargetTransform.GetRotation().Rotator());
		}

		/* 암살 시작 */
		UDJAbilitySystemComponent* ASC = GetDJAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			ASC->AddLooseGameplayTag(Tag_Status_Assassinationing);
		}
		UAbilityTask_PlayMontageAndWait* AssassinationMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("Assassination"), AssassinationMontages[CurrentMontageindex].AssassinationMontage, 1.f, NAME_None);

		if(Character->GetMesh()->GetAnimInstance()->OnPlayMontageNotifyBegin.Contains(this, FName("MontageBeginNotifyCallback")) == false )
			Character->GetMesh()->GetAnimInstance()->OnPlayMontageNotifyBegin.AddDynamic(this, &ThisClass::MontageBeginNotifyCallback);
		AssassinationMontage->OnInterrupted.AddDynamic(this, &UDJGA_Assassination::EndFunction);
		AssassinationMontage->OnCancelled.AddDynamic(this, &UDJGA_Assassination::EndFunction);
		AssassinationMontage->OnCompleted.AddDynamic(this, &UDJGA_Assassination::EndFunction);
		AssassinationMontage->ReadyForActivation();
	}
}

void UDJGA_Assassination::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (TriggerRadius)
	{
		TriggerRadius->Destroy();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDJGA_Assassination::EndFunction()
{
	Character->SetActorEnableCollision(true);
	UDJAbilitySystemComponent* ASC = GetDJAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		ASC->RemoveLooseGameplayTag(Tag_Status_Assassinationing);
	}
}

void UDJGA_Assassination::MontageBeginNotifyCallback(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	//해당 타이밍에 타겟이 피격 모션발생하도록 이벤트 전송
	if (NotifyName == FName(TEXT("StartVictimAnim")))
	{
		FGameplayEventData PayloadData;
		FGameplayEffectContext* context = new FGameplayEffectContext();
		context->SetAbility(this);
		PayloadData.ContextHandle = context;
		PayloadData.Instigator = CurrentActorInfo->AvatarActor.Get();

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor, VictimAbilityTriggerTag, PayloadData);
	}
}

void UDJGA_Assassination::OnGetTargetCallback(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	TargetActor = nullptr;	//타겟 초기화

	//누군가 범위 안에 들어왔으면 각도 체크하여 암살 가능 범위중에
	//정면에 가장 가까운 액터를 찾아서 암살 대상으로 적용
	if (UAbilitySystemBlueprintLibrary::TargetDataHasActor(TargetDataHandle, 0))
	{
		FVector AssassinPos = CurrentActorInfo->AvatarActor->GetActorLocation();
		FVector ForwardDir = CurrentActorInfo->AvatarActor->GetActorForwardVector();
		float CurrentAngle = MaxAngle;
		AActor* Target = nullptr;

		TArray<AActor*> OverlapActors = UAbilitySystemBlueprintLibrary::GetActorsFromTargetData(TargetDataHandle, 0);
		for (AActor* Overlap : OverlapActors)
		{
			UAbilitySystemComponent* TargetASC =  UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Overlap);
			if (TargetASC == nullptr)
				continue;
			if (TargetASC->HasMatchingGameplayTag(Tag_Status_Facing))	//타겟이 플레이어를 인지하고 있다면 실패
				continue;

			FVector OverlapDir = Overlap->GetActorLocation() - AssassinPos;
			if (FMath::Abs(OverlapDir.Z) > 40)	//높이 차가 크면 암살 안함.
				continue;
			OverlapDir.Z = 0;
			OverlapDir.Normalize();

			float tempAngle = FMath::RadiansToDegrees( acosf(ForwardDir.Dot(OverlapDir)));	//각도차
			if (tempAngle < CurrentAngle)
			{
				CurrentAngle = tempAngle;
				TargetActor = Overlap;
				TargetDir = OverlapDir;
			}
		}
	}

	if (TargetActor)
	{
		//타겟 위에 위젯 생성.

	}
}

void UDJGA_Assassination::ProcessAssassin()
{
}
