// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PawnData.generated.h"

USTRUCT(BlueprintType)
struct FInputMappingContextAndPriority
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AssetBundles = "Client,Server"))
	TSoftObjectPtr<class UInputMappingContext> InputMapping;

	// Higher priority input mappings will be prioritized over mappings with a lower priority.
	UPROPERTY(EditAnywhere, Category = "Input")
	int32 Priority = 0;

	///** If true, then this mapping context will be registered with the settings when this game feature action is registered. */
	//UPROPERTY(EditAnywhere, Category = "Input")
	//bool bRegisterWithSettings = true;
};

/**
 * 
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "Lyra Pawn Data", ShortTooltip = "Data asset used to define a Pawn."))
class DJGAME_API UPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPawnData(const FObjectInitializer& ObjectInitializer);

public:
	// Ability sets to grant to this pawn's ability system.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DJ|Abilities")
	TArray<TObjectPtr<class UDJAbilitySet>> AbilitySets;

	// Input configuration used by player controlled pawns to create input mappings and bind input actions.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DJ|Input")
	TObjectPtr<class UDJInputData> InputData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DJ|Mapping")
	TArray<FInputMappingContextAndPriority> InputMappings;


};
