// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GA/DJGameplayAbility.h"

#include "LyraGameplayAbility_FromEquipment.generated.h"

class ULyraEquipmentInstance;
class ULyraInventoryItemInstance;

/**
 * ULyraGameplayAbility_FromEquipment
 *
 * An ability granted by and associated with an equipment instance
 */
UCLASS()
class ULyraGameplayAbility_FromEquipment : public UDJGameplayAbility
{
	GENERATED_BODY()

public:

	ULyraGameplayAbility_FromEquipment(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category="DJ|Ability")
	ULyraEquipmentInstance* GetAssociatedEquipment() const;

	UFUNCTION(BlueprintCallable, Category = "DJ|Ability")
	ULyraInventoryItemInstance* GetAssociatedItem() const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

};
