// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "TargetActor_TickableRadius.generated.h"

class UGameplayAbility;

/**
 * 
 */
UCLASS(Blueprintable, notplaceable)
class DJGAME_API ATargetActor_TickableRadius : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()
	
public:
	ATargetActor_TickableRadius(const FObjectInitializer& ObjectInitializer);

	//~ AActor 재정의
	virtual void Tick(float DeltaSeconds) override;
	//~ AActor 끝

	//~ AGameplayAbilityTargetActor 재정의
	virtual void StartTargeting(UGameplayAbility* Ability) override;
	virtual void ConfirmTargetingAndContinue() override;
	//~ AGameplayAbilityTargetActor 끝

	/** Radius of target acquisition around the ability's start location. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Radius)
	float Radius;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Radius)
	FVector Offset;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Radius)
	TArray<AActor*> IgnoreActors;

protected:

	TArray<FHitResult> PerformOverlap(const FVector& Origin);

	FGameplayAbilityTargetDataHandle MakeTargetData(const TArray<FHitResult> HitResults) const;

private:
	bool bConfirm = false;
};
