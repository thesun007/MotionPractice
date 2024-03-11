// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DJCharacterMovementComponent.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FLyraCharacterGroundInfo
{
	GENERATED_BODY()

	FLyraCharacterGroundInfo()
		: LastUpdateFrame(0)
		, GroundDistance(0.0f)
	{}

	uint64 LastUpdateFrame;

	UPROPERTY(BlueprintReadOnly)
	FHitResult GroundHitResult;

	UPROPERTY(BlueprintReadOnly)
	float GroundDistance;
};

UCLASS()
class DJGAME_API UDJCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	UDJCharacterMovementComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	float GetMaxJogSpeed() { return CustomMaxJogSpeed; }
	float GetMaxWalkSpeed() { return CustomMaxWalkSpeed; }
	UFUNCTION(BlueprintCallable, Category = "DJ|CharacterMovement")
	const FLyraCharacterGroundInfo& GetGroundInfo();

protected:
	UPROPERTY(EditDefaultsOnly, Category = CustomSetting )
	float CustomMaxJogSpeed;
	UPROPERTY(EditDefaultsOnly, Category = CustomSetting)
	float CustomMaxWalkSpeed;

	FLyraCharacterGroundInfo CachedGroundInfo;
};
