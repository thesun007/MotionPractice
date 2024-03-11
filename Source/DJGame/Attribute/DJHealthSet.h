// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/DJAttributeSet.h"

#include "DJHealthSet.generated.h"

/**
 * 
 */
UCLASS()
class DJGAME_API UDJHealthSet : public UDJAttributeSet
{
	GENERATED_BODY()
	
public:
	UDJHealthSet();

	ATTRIBUTE_ACCESSORS(UDJHealthSet, Health);
	ATTRIBUTE_ACCESSORS(UDJHealthSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(UDJHealthSet, Healing);
	ATTRIBUTE_ACCESSORS(UDJHealthSet, Damage);

	// Delegate when health changes due to damage/healing, some information may be missing on the client
	mutable FDJAttributeEvent OnHealthChanged;

	// Delegate when max health changes
	mutable FDJAttributeEvent OnMaxHealthChanged;

	// Delegate to broadcast when the health attribute reaches zero
	mutable FDJAttributeEvent OnOutOfHealth;

protected:
	//~ UAttributeSet 재정의
	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	//~ UAttributeSet 재정의 끝

	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

private:
	// The current health attribute.  The health will be capped by the max health attribute.  Health is hidden from modifiers so only executions can modify it.
	UPROPERTY(BlueprintReadOnly, /*ReplicatedUsing = OnRep_Health, */Category = "DJ|Health", Meta = (HideFromModifiers, AllowPrivateAccess = true))
	FGameplayAttributeData Health;

	// The current max health attribute.  Max health is an attribute since gameplay effects can modify it.
	UPROPERTY(BlueprintReadOnly, /*ReplicatedUsing = OnRep_MaxHealth,*/ Category = "DJ|Health", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxHealth;

	// Used to track when the health reaches 0.
	bool bOutOfHealth;

	// Store the health before any changes 
	float MaxHealthBeforeAttributeChange;
	float HealthBeforeAttributeChange;


	// -------------------------------------------------------------------
	//	Meta Attribute (please keep attributes that aren't 'stateful' below 
	// -------------------------------------------------------------------

	// Incoming healing. This is mapped directly to +Health
	UPROPERTY(BlueprintReadOnly, Category = "DJ|Health", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Healing;

	// Incoming damage. This is mapped directly to -Health
	UPROPERTY(BlueprintReadOnly, Category = "DJ|Health", Meta = (HideFromModifiers, AllowPrivateAccess = true))
	FGameplayAttributeData Damage;
};
