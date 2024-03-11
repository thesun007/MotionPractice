// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/DJCharacterBase.h"
#include "AbilitySystemInterface.h"
#include "System/GameplayTagStack.h"
#include "DJCharacterNPC.generated.h"

class UDJAbilitySystemComponent;

/**
 * 
 */
UCLASS()
class DJGAME_API ADJCharacterNPC : public ADJCharacterBase, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	ADJCharacterNPC(const FObjectInitializer& ObjectInitializer);

	/*UFUNCTION(BlueprintCallable, Category = "DJ|Character")
	class ADJPlayerController* GetDJPlayerController() const;

	UFUNCTION(BlueprintCallable, Category = "DJ|Character")
	class ADJPlayerState* GetDJPlayerState() const;*/

	UFUNCTION(BlueprintCallable, Category = "DJ|NPC")
	UDJAbilitySystemComponent* GetDJAbilitySystemComponent() const { return AbilitySystemComponent; }
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }
	void SetPawnData(const class UPawnData* InPawnData);

	//~ACharacter 함수 재정의
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	//~끝

protected:
	//~AActor interface
	//virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	//virtual void BeginPlay() override;
	//~End of AActor interface

	//~ Pawn 함수 재정의
	//virtual void PossessedBy(AController* NewController) override;
	//~ 끝 	

private:
	//
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DJ|Assassination", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USkeletalMeshComponent> AssassinRef;

	// The ability system component sub-object used by player characters.
	UPROPERTY(VisibleAnywhere, Category = "DJ|NPC")
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
