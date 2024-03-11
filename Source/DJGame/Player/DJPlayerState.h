// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "System/GameplayTagStack.h"
#include "DJPlayerState.generated.h"

class ADJPlayerController;
class UDJAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class DJGAME_API ADJPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	ADJPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "DJ|PlayerState")
	ADJPlayerController* GetDJPlayerController() const;

	//~ IAbilitySystemInterface 재정의
	UFUNCTION(BlueprintCallable, Category = "DJ|PlayerState")
	UDJAbilitySystemComponent* GetDJAbilitySystemComponent() const { return AbilitySystemComponent; }
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override ;
	//~ IAbilitySystemInterface 끝

	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }
	void SetPawnData(const class UPawnData* InPawnData);

	// Adds a specified number of stacks to the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Teams)
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Removes a specified number of stacks from the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Teams)
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Returns the stack count of the specified tag (or 0 if the tag is not present)
	UFUNCTION(BlueprintCallable, Category = Teams)
	int32 GetStatTagStackCount(FGameplayTag Tag) const;

	// Returns true if there is at least one stack of the specified tag
	UFUNCTION(BlueprintCallable, Category = Teams)
	bool HasStatTag(FGameplayTag Tag) const;

	//~PlayerState 함수 재정의
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	//~끝

protected:
	//~AActor interface
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	//~End of AActor interface

private:

	// The ability system component sub-object used by player characters.
	UPROPERTY(VisibleAnywhere, Category = "DJ|PlayerState")
	TObjectPtr<UDJAbilitySystemComponent> AbilitySystemComponent;

	//PawnData 지정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<const class UPawnData> PawnData;

	// Health attribute set used by this actor.
	UPROPERTY()
	TObjectPtr<const class UDJHealthSet> HealthSet;
	//// Combat attribute set used by this actor.
	//UPROPERTY()
	//TObjectPtr<const class UDJCombatSet> CombatSet;

	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTags;

};
