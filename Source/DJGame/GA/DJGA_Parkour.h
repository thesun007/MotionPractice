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
	//~ GameAbility �Լ� ������
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	//~ ��

private:
	void ProcessInNormal(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);
	
	void ProcessInJump(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);
	void TryMantling(FHitResult& Result);
	void TryWallRun(FHitResult& Result, bool Left);

	void ProcessMontage(class UAnimMontage* _Montage, float StartTime = 0.0f);
	
	UFUNCTION()
	void MontageEndNotifyCallback(FName NotifyName, const struct FBranchingPointNotifyPayload& BranchingPointPayload);
	
	//���� ���μ����� ��, ������ �½�ũ �ݹ��Լ�
	UFUNCTION()
	void OverlapCallback(const FGameplayAbilityTargetDataHandle& TargetData);

	//(��Ÿ�� ���н�)(�½�ũ �����) ���� �Լ�
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

	//���� ������

	UPROPERTY()
	TObjectPtr<ADJCharacterBase> Character;
	FVector VaultStartPos;
	FVector VaultMiddlePos;
	FVector VaultEndPos;
	FRotator VaultStartRot;
	EHeightType CurrentHeight;
};
