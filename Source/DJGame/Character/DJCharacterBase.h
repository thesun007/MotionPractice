// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagAssetInterface.h"
#include "DJCharacterBase.generated.h"



UENUM(BlueprintType)
enum class EFullBodyPose : uint8
{
	UnArmed,
	Pistol,
	Rifle,
	Shotgun
};

UCLASS(abstract)
class DJGAME_API ADJCharacterBase : public ACharacter, public IAbilitySystemInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ADJCharacterBase(const FObjectInitializer& ObjectInitializer);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = Animation)
	void SetAnimLayer(EFullBodyPose pose);

	UFUNCTION(BlueprintCallable, Category = "DJ|CharacterBase")
	UDJAbilitySystemComponent* GetDJAbilitySystemComponent() const { return ASC; }
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, Category = "MotionWarping")
		UMotionWarpingComponent* GetMotionWarpingComponent() {return MotionWarpingComponent;}

	//~ IGameplayTagAssetInterface ������
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	//~ IGameplayTagAssetInterface ��

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PostInitializeComponents() override;
	
	virtual void InitializeASC() PURE_VIRTUAL(ThisClass::InitializeASC, ;);

protected:
	// The ability system component sub-object used by player characters.
	UPROPERTY(VisibleAnywhere, Category = "DJ|", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UDJAbilitySystemComponent> ASC;

private:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animation, Meta = (AllowPrivateAccess = "true"))
	TMap<EFullBodyPose, TSubclassOf<UAnimInstance>> OtherAnimLayers;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Motion Warping", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UMotionWarpingComponent> MotionWarpingComponent;
};
