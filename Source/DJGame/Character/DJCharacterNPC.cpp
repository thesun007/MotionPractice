// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/DJCharacterNPC.h"
#include "GAS/DJAbilitySystemComponent.h"
#include "Character/PawnData.h"
#include "GAS/DJAbilitySet.h"
#include "Net/UnrealNetwork.h"
#include "DJCharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemBlueprintLibrary.h"
//#include "Components/SkeletalMeshComponent.h"
#include "DJGame.h"

ADJCharacterNPC::ADJCharacterNPC(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UDJCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	ASC = ObjectInitializer.CreateDefaultSubobject<UDJAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	/*AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);*/
	
	AssassinRef = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("AssassinRef"));
	AssassinRef->SetupAttachment(RootComponent);
	//AssassinRef->SetSkeletalMesh(GetMesh()->GetSkeletalMeshAsset());
	AssassinRef->SetRelativeLocationAndRotation(FVector(-108.f, 0.0f, -90.0f), FRotator(0.0f, -90.0f, 0.0f));
	AssassinRef->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	AssassinRef->bHiddenInGame = true;
	AssassinRef->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	//********************** Pawn Data ***********************//
	static ConstructorHelpers::FObjectFinder<UPawnData> PawnDataRef(TEXT("/Script/DJGame.PawnData'/Game/DJGame/Data/DA_PawnData_Default.DA_PawnData_Default'"));
	if (nullptr != PawnDataRef.Object)
	{
		PawnData = PawnDataRef.Object;
	}
}

void ADJCharacterNPC::SetPawnData(const UPawnData* InPawnData)
{
	check(InPawnData);

	/*if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}*/

	if (PawnData)
	{
		DJ_LOG(DJLog, Error, TEXT("Trying to set PawnData [%s] on player NPC [%s] that already has valid PawnData [%s]."), *GetNameSafe((UObjectBaseUtility*)InPawnData), *GetNameSafe(this), *GetNameSafe((UObjectBaseUtility*)PawnData));
		return;
	}

	//MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PawnData, this);
	PawnData = InPawnData;

	for (const UDJAbilitySet* AbilitySet : PawnData->AbilitySets)
	{
		if (AbilitySet)
		{
			AbilitySet->GiveToAbilitySystem(ASC, nullptr, this);
		}
	}

	//UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, NAME_LyraAbilityReady);
	//ForceNetUpdate();
}

void ADJCharacterNPC::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, StatTags);
}

void ADJCharacterNPC::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	InitializeASC();
}

void ADJCharacterNPC::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	//ai 컨트롤러와 연결된 경우 호출됨.
}

void ADJCharacterNPC::InitializeASC()
{
	ASC->InitAbilityActorInfo(this, this);

	for (const UDJAbilitySet* AbilitySet : PawnData->AbilitySets)
	{
		if (AbilitySet)
		{
			AbilitySet->GiveToAbilitySystem(ASC, nullptr, this);
		}
	}
}
