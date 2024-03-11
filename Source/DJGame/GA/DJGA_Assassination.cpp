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
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;	//�ӽ�
	
	AbilityTags.AddTag(TAG_AbilityType_Action_Assassination);
	FAbilityTriggerData trigger;
	trigger.TriggerTag = TAG_InputTag_Assassination;
	AbilityTriggers.Add(trigger);

	VictimAbilityTriggerTag = TAG_InputTag_AssassinationVictim;	//����� �����Ƽ ���� �±�
}

void UDJGA_Assassination::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// �ʱ�ȭ
	Character = GetDJCharacterFromActorInfo();

	//**�ϻ� ���� ����**//
	TriggerRadius = GetWorld()->SpawnActor<AGameplayAbilityTargetActor_Radius>(ActorInfo->AvatarActor.Get()->GetActorLocation(), ActorInfo->AvatarActor.Get()->GetActorRotation());

	//Ʈ���� ����
	TriggerRadius->Radius = Radius;
	TriggerRadius->bDebug = bDebug;
	TriggerRadius->bDestroyOnConfirmation = false;		//��� Ʈ���� �õ��ϱ� ���� �ش� �ɼ� ��Ȱ��ȭ
	TriggerRadius->StartLocation = MakeTargetLocationInfoFromOwnerActor();	//���� ������ ��ġ�� ������ġ�� ����

	//���� �����
	FGameplayTargetDataFilterHandle filterHandle;
	FGameplayTargetDataFilter filter;
	filter.SelfActor = CurrentActorInfo->AvatarActor.Get();
	filter.RequiredActorClass = ADJCharacterBase::StaticClass();
	filter.SelfFilter = ETargetDataFilterSelf::TDFS_NoSelf;
	filterHandle.Filter = MakeShared<FGameplayTargetDataFilter, ESPMode::ThreadSafe>(filter);
	TriggerRadius->Filter = filterHandle;
	
	//(�ϻ� ���� Ʈ����)�����Ƽ �½�ũ ����
	UDJAT_Trigger* TriggerTask = UDJAT_Trigger::CreateTriggerTaskUsingActor(this, TEXT("AssassinationTask"),EGameplayTargetingConfirmation::Instant, true, TriggerRadius);	//TargetActor�� Trigger �½�ũ �� �Բ� �Ҹ�
	TriggerTask->ValidData.AddDynamic(this, &UDJGA_Assassination::OnGetTargetCallback);
	TriggerTask->ReadyForActivation();
	
}

void UDJGA_Assassination::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	//Super::InputPressed(Handle, ActorInfo, ActivationInfo);
	//*�ϻ� ���� ��� �ִٸ� �ϻ� ��� ����*//

	if (Character && TargetActor && AssassinationMontages.Num()>0)
	{
		/* ���� ground ���� ���� */
		FTransform TargetTransform = TargetActor->GetActorTransform();	//Ÿ�� Ʈ������
		FVector AssassinToTarget = Character->GetActorLocation() - TargetTransform.GetLocation();	// Ÿ�� ���� �ϻ��� ��ġ

		/* ���� ������ �ϻ� Ÿ�� ���ϱ� */
		EAssassinationType GaolAssassinationType;		// �ϻ�Ÿ�� ���� ����
		FVector AssassinToTargetDir = AssassinToTarget;	// Ÿ�� ���� �ϻ��� ����
		AssassinToTargetDir.Z = 0;

		FRotator DeltaRotation = (FRotationMatrix::MakeFromX(AssassinToTargetDir).Rotator() - TargetTransform.Rotator());	// Ÿ�� ����, �ϻ��� ��ġ�� ����.
		DeltaRotation.Normalize();

		//UE_LOG(LogTemp, Log, TEXT("Target : %f,  TargetToAssassin : %f"), TargetTransform.Rotator().Yaw, DeltaRotation.Yaw);
		if (-50. < DeltaRotation.Yaw && DeltaRotation.Yaw < 50.)
			GaolAssassinationType = EAssassinationType::front;
		else if(DeltaRotation.Yaw < -130 || DeltaRotation.Yaw > 130)
			GaolAssassinationType = EAssassinationType::Back;
		else
			GaolAssassinationType = EAssassinationType::Side;

		/* �ϻ� ��Ÿ�� ���� */
		TArray< FAssassinationMontageData> ListMontages;
		for (FAssassinationMontageData data : AssassinationMontages)
		{
			if (GaolAssassinationType == data.Type)
				ListMontages.Add(data);
		}

		//��Ÿ�� ������ ���
		if (ListMontages.IsEmpty())
		{
			return;
		}

		//Character->SetActorEnableCollision(false);
		CurrentMontageindex = FMath::RandRange(0, ListMontages.Num() - 1);


		FAssassinationMontageData SelectedMontageData = ListMontages[CurrentMontageindex];	// ���� �ϻ� ��Ÿ��

		/* ��Ÿ�� ���� �õ� */
		if (CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo) == false)
		{
			return;
		}

		/* ��ǿ��� ���� */
		FVector AbsolutePos = TargetTransform.TransformPosition(SelectedMontageData.SyncPosition);	// �ϻ��ڰ� ��ġ�ؾ� �� ���밪
		UMotionWarpingComponent* MWC = Character->GetMotionWarpingComponent();
		if (MWC)
		{
			MWC->AddOrUpdateWarpTargetFromLocationAndRotation(FName(TEXT("StartPos")), AbsolutePos, TargetTransform.GetRotation().Rotator());
		}

		/* �ϻ� ���� */
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
	//�ش� Ÿ�ֿ̹� Ÿ���� �ǰ� ��ǹ߻��ϵ��� �̺�Ʈ ����
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
	TargetActor = nullptr;	//Ÿ�� �ʱ�ȭ

	//������ ���� �ȿ� �������� ���� üũ�Ͽ� �ϻ� ���� �����߿�
	//���鿡 ���� ����� ���͸� ã�Ƽ� �ϻ� ������� ����
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
			if (TargetASC->HasMatchingGameplayTag(Tag_Status_Facing))	//Ÿ���� �÷��̾ �����ϰ� �ִٸ� ����
				continue;

			FVector OverlapDir = Overlap->GetActorLocation() - AssassinPos;
			if (FMath::Abs(OverlapDir.Z) > 40)	//���� ���� ũ�� �ϻ� ����.
				continue;
			OverlapDir.Z = 0;
			OverlapDir.Normalize();

			float tempAngle = FMath::RadiansToDegrees( acosf(ForwardDir.Dot(OverlapDir)));	//������
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
		//Ÿ�� ���� ���� ����.

	}
}

void UDJGA_Assassination::ProcessAssassin()
{
}
