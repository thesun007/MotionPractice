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
	//~ GameAbility �Լ� ������
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	//~ ��

private:
	//
	UFUNCTION(Server, Reliable)
	void SendInfo(FVector _Dir, class UAnimMontage* _Montage);

	void ProcessMontageNTask(FVector _Dir, class UAnimMontage* _Montage);
	
	//(��Ÿ�� ���н�)(�½�ũ �����) ���� �Լ�
	UFUNCTION()
	void EndFunction();

	//�̵� �½�ũ ������ �� ������ �ݹ� �Լ�
	UFUNCTION()
	void OnTaskFinishCallback();

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim", meta =(AllowPrivateAccess = "true"))
	FDashMongtageDir DashMontage;

	UPROPERTY()
	TObjectPtr<class UAnimMontage> Montage;

	//��Ʈ ��� �� �μ�
	UPROPERTY(EditAnywhere, Category = "Force", meta = (AllowPrivateAccess = "true"))
	float Strength;
	UPROPERTY(EditAnywhere, Category = "Force", meta = (AllowPrivateAccess = "true"))
	float Duration;
};
