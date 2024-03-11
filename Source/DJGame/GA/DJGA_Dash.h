// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GA/DJGameplayAbility.h"
#include "DJGA_Dash.generated.h"

USTRUCT(BlueprintType)
struct FDashMongtageDir
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> Forward;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TObjectPtr<class UAnimMontage> Backward;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TObjectPtr<class UAnimMontage> Left;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TObjectPtr<class UAnimMontage> Right;
};
/**
 * 
 */
UCLASS()
class DJGAME_API UDJGA_Dash : public UDJGameplayAbility
{
	GENERATED_BODY()
	
public:
	UDJGA_Dash(const FObjectInitializer& ObjectInitializer);
protected:
	//~ GameAbility 함수 재정의
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	//~ 끝

private:
	//
	UFUNCTION(Server, Reliable)
	void SendInfo(FVector _Dir, class UAnimMontage* _Montage);

	void ProcessMontageNTask(FVector _Dir, class UAnimMontage* _Montage);
	
	//(몽타주 실패시)(태스크 종료시) 종료 함수
	UFUNCTION()
	void EndFunction();

	//이동 태스크 끝났을 때 마무리 콜백 함수
	UFUNCTION()
	void OnTaskFinishCallback();

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim", meta =(AllowPrivateAccess = "true"))
	FDashMongtageDir DashMontage;

	UPROPERTY()
	TObjectPtr<class UAnimMontage> Montage;

	//루트 모션 힘 인수
	UPROPERTY(EditAnywhere, Category = "Force", meta = (AllowPrivateAccess = "true"))
	float Strength;
	UPROPERTY(EditAnywhere, Category = "Force", meta = (AllowPrivateAccess = "true"))
	float Duration;
};
