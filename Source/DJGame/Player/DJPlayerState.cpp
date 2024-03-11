// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/DJPlayerState.h"
#include "DJPlayerController.h"
#include "GAS/DJAbilitySystemComponent.h"
#include "Character/PawnData.h"
#include "GAS/DJAbilitySet.h"
#include "Net/UnrealNetwork.h"

#include "DJGame.h"

ADJPlayerState::ADJPlayerState(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UDJAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	/*AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);*/

	//********************** Pawn Data ***********************//
	static ConstructorHelpers::FObjectFinder<UPawnData> PawnDataRef(TEXT("/Script/DJGame.PawnData'/Game/DJGame/Data/DA_PawnData_Default.DA_PawnData_Default'"));
	if (nullptr != PawnDataRef.Object)
	{
		PawnData = PawnDataRef.Object;
	}
}

ADJPlayerController* ADJPlayerState::GetDJPlayerController() const
{
	return Cast<ADJPlayerController>(GetOwner());
}

UAbilitySystemComponent* ADJPlayerState::GetAbilitySystemComponent() const
{
	return GetDJAbilitySystemComponent(); 
}

void ADJPlayerState::SetPawnData(const UPawnData* InPawnData)
{
	check(InPawnData);

	/*if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}*/

	if (PawnData)
	{
		DJ_LOG(DJLog, Error, TEXT("Trying to set PawnData [%s] on player state [%s] that already has valid PawnData [%s]."), *GetNameSafe((UObjectBaseUtility*)InPawnData), *GetNameSafe(this), *GetNameSafe((UObjectBaseUtility*)PawnData));
		return;
	}

	//MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PawnData, this);
	PawnData = InPawnData;

	for (const UDJAbilitySet* AbilitySet : PawnData->AbilitySets)
	{
		if (AbilitySet)
		{
			AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);
		}
	}

	//UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, NAME_LyraAbilityReady);
	//ForceNetUpdate();
}

void ADJPlayerState::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.AddStack(Tag, StackCount);
}

void ADJPlayerState::RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.RemoveStack(Tag, StackCount);
}

int32 ADJPlayerState::GetStatTagStackCount(FGameplayTag Tag) const
{
	return StatTags.GetStackCount(Tag);
}

bool ADJPlayerState::HasStatTag(FGameplayTag Tag) const
{
	return StatTags.ContainsTag(Tag);
}

void ADJPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, StatTags);
}

void ADJPlayerState::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void ADJPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}
