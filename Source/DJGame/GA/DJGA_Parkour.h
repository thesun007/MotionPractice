// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GA/DJGameplayAbility.h"
#include "DJGA_Parkour.generated.h"

UENUM(BlueprintType)
enum class EHeightType : uint8
{
	Low,
	Middle,
	High
};

USTRUCT(BlueprintType)
struct FMontageArray
{
	GENERATED_BODY()

	FMontageArray() {};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TObjectPtr<class UAnimMontage>> Datas;	// 0 : climb , 1: short vault, 2: middle vault
};

/**
 * 
 */
UCLASS()
class DJGAME_API UDJGA_Parkour : public UDJGameplayAbility
{
	GENERATED_BODY()
	
public:
	UDJGA_Parkour(const FObjectInitializer& ObjectInitializer);

protected:
	//~ GameAbility 함수 재정의
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	//~ 끝

private:
	void ProcessInNormal(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);
	
	void ProcessInJump(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);
	void TryMantling(FHitResult& Result);
	void TryWallRun(FHitResult& Result, bool Left);

	void ProcessMontage(class UAnimMontage* _Montage, float StartTime = 0.0f);
	
	UFUNCTION()
	void MontageEndNotifyCallback(FName NotifyName, const struct FBranchingPointNotifyPayload& BranchingPointPayload);
	
	//점프 프로세스일 때, 오버랩 태스크 콜백함수
	UFUNCTION()
	void OverlapCallback(const FGameplayAbilityTargetDataHandle& TargetData);

	//(몽타주 실패시)(태스크 종료시) 종료 함수
	UFUNCTION()
	void EndFunction();

private:
	// 0 : climb , 1: short vault, 2: middle vault
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim", meta = (AllowPrivateAccess = "true"))
	TMap<EHeightType, FMontageArray> Montages;

	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	uint8 bDebug:1 ;
	
	UPROPERTY(EditDefaultsOnly, Category = "Height")
	float MaxHeight = 200;
	UPROPERTY(EditDefaultsOnly, Category = "Height")
	float MiddleHeight = 170;
	UPROPERTY(EditDefaultsOnly, Category = "Height")
	float LowHeight = 100;

	//내부 로직용

	UPROPERTY()
	TObjectPtr<ADJCharacterBase> Character;
	FVector VaultStartPos;
	FVector VaultMiddlePos;
	FVector VaultEndPos;
	FRotator VaultStartRot;
	EHeightType CurrentHeight;
};
