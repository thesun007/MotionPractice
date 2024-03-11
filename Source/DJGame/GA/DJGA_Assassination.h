// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GA/DJGameplayAbility.h"
#include "Tag/DJGameplayTags.h"
#include "DJGA_Assassination.generated.h"

struct FGameplayTag;

UENUM(BlueprintType)
enum class EAssassinationType
{
	front,
	Back,
	Side,
	Aerial
};

USTRUCT(BlueprintType)
struct FAssassinationMontageData
 {
	 GENERATED_BODY()
	 
	 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	 TObjectPtr<class UAnimMontage> AssassinationMontage;
	
	 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	 EAssassinationType Type;

	 //this is local position on target/.
	 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	 FVector SyncPosition;
 };
/**
 * 
 */
UCLASS()
class DJGAME_API UDJGA_Assassination : public UDJGameplayAbility
{
	GENERATED_BODY()
	
public:
	UDJGA_Assassination(const FObjectInitializer& ObjectInitializer);

	int32 GetMontageIndex() const { return CurrentMontageindex; }

protected:
	//~ GameAbility 함수 재정의
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~ 끝

	//(몽타주 실패시)(태스크 종료시) 종료 함수
	UFUNCTION()
	void EndFunction();
	UFUNCTION()
	void MontageBeginNotifyCallback(FName NotifyName, const struct FBranchingPointNotifyPayload& BranchingPointPayload);

private:
	UFUNCTION()
	void OnGetTargetCallback(const struct FGameplayAbilityTargetDataHandle& TargetDataHandle);
	void ProcessAssassin();

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AssassinationAnimation" , meta = (AllowPrivateAccess = "true"))
	TArray<FAssassinationMontageData> AssassinationMontages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AssassinationSetting", meta = (AllowPrivateAccess = "true"))
	float Radius = 120.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AssassinationSetting", meta = (AllowPrivateAccess = "true"))
	float MaxAngle = 45.f;
	/*UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<class UAnimMontage>> VictimMontages;*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AssassinationSetting", meta = (AllowPrivateAccess = "true"))
	FGameplayTag VictimAbilityTriggerTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AssassinationSetting", meta = (AllowPrivateAccess = "true"))
	uint8 bDebug :1 = false;

	UPROPERTY()
	TObjectPtr<class AGameplayAbilityTargetActor_Radius> TriggerRadius		;	//암살 대상 감시용

	/*UPROPERTY()
	TObjectPtr<class AGameplayAbilityTargetActor_Radius> TriggerRadius;
	UPROPERTY()
	TObjectPtr<class UDJAT_Trigger> TriggerTask*/

	UPROPERTY()
	TObjectPtr<ADJCharacterBase> Character;
	AActor* TargetActor = nullptr;
	FVector TargetDir;
	int32 CurrentMontageindex = 0;
};
