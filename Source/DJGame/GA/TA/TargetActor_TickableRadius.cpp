// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/TA/TargetActor_TickableRadius.h"
#include "Abilities/GameplayAbility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TargetActor_TickableRadius)	//컴파일 모듈 인라인화.  성능 높혀줌.

ATargetActor_TickableRadius::ATargetActor_TickableRadius(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	ShouldProduceTargetDataOnServer = true;
	bDestroyOnConfirmation = false;
}

void ATargetActor_TickableRadius::Tick(float DeltaSeconds)
{
	if (bConfirm)
	{
		check(ShouldProduceTargetData());
		if (SourceActor)
		{
			FVector Origin = StartLocation.GetTargetingTransform().GetLocation() + Offset;
			FGameplayAbilityTargetDataHandle Handle = MakeTargetData(PerformOverlap(Origin));
			TargetDataReadyDelegate.Broadcast(Handle);
		}
	}
}

void ATargetActor_TickableRadius::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);
	SourceActor = Ability->GetCurrentActorInfo()->AvatarActor.Get(); 
}

void ATargetActor_TickableRadius::ConfirmTargetingAndContinue()
{
	bConfirm = true;
}

TArray<FHitResult> ATargetActor_TickableRadius::PerformOverlap(const FVector& Origin)
{
	bool bTraceComplex = false;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(RadiusTargetingOverlap), bTraceComplex);
	Params.bReturnPhysicalMaterial = false;
	Params.AddIgnoredActors(IgnoreActors);

	TArray<FHitResult> HitResults;

	SourceActor->GetWorld()->SweepMultiByChannel(HitResults, Origin, Origin, FQuat::Identity, ECollisionChannel::ECC_Visibility, FCollisionShape::MakeSphere(Radius), Params);

	for (int i =0; i< HitResults.Num(); ++i)
	{
		AActor* FetchActor = HitResults[i].GetHitObjectHandle().FetchActor();
		if (!Filter.FilterPassesForActor(FetchActor))
		{
			HitResults.RemoveAt(i);
		}
	}

#if ENABLE_DRAW_DEBUG
	if (bDebug)
	{
		FColor color = FColor::Red;
		if (HitResults.Num() > 0)
			color = FColor::Green;

		DrawDebugSphere(GetWorld(), Origin, Radius, 10, color, false);
	}
#endif // ENABLE_DRAW_DEBUG

	return HitResults;
}

FGameplayAbilityTargetDataHandle ATargetActor_TickableRadius::MakeTargetData(const TArray<FHitResult> HitResults) const
{
	if (OwningAbility)
	{
		/** Use the source location instead of the literal origin */
		return StartLocation.MakeTargetDataHandleFromHitResults(OwningAbility, HitResults);
	}

	return FGameplayAbilityTargetDataHandle();
}
