// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "DJAT_WallRun.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTaskResultDelegate);

enum class EWallRunState : uint8
{
	Loop,
	End
};

/**
 * 
 */
UCLASS()
class DJGAME_API UDJAT_WallRun : public UAbilityTask
{
	GENERATED_BODY()

public:
	UDJAT_WallRun();

	//~ UGameplayTask ������
	virtual void TickTask(float DeltaTime) override;	// ticking Target Actor ����

	UFUNCTION(BlueprintCallable, Category = "Ability Tasks", Meta = (DisplayName = "TriggerTask",
	HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "True"))
		static UDJAT_WallRun* CreateWallRunTask(UGameplayAbility* OwningAbility, FName TaskInstanceName, FVector StartWallPoint, FVector StartWallNormal,
			bool Left, float _GapOffsetWithWall);

private:
	//~ UAbilityTask ������
	virtual void Activate() override;						
	virtual void OnDestroy(bool bInOwnerFinished) override;		//EndTask()�� ����Ǹ� ȣ��� ��.
	//~ UAbilityTask ��

	UFUNCTION()
	void MontageBeginNotifyCallback(FName NotifyName, const struct FBranchingPointNotifyPayload& BranchingPointPayload);

	void OnCompleted();
	void OnCanceled();

public:
	UPROPERTY(BlueprintAssignable)
	FTaskResultDelegate	Completed;

	UPROPERTY(BlueprintAssignable)
	FTaskResultDelegate	Canceled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	uint8	bDebug = false;

private:
	UPROPERTY()
	TObjectPtr<class ADJCharacterBase> Character;
	UPROPERTY()
	TObjectPtr<class UDJAnimInstance> AnimInstance;
	//�������̽��� ������ ������ ���.
	UPROPERTY()
	TScriptInterface<class IAnimSourceSetInterface> AnimSourceSetInterface;

	bool bLeft;		//���� ��������
	float MaxSpeed;
	float CurrentSpeed;
	float CurrentVirSpeed;
	float GapOffsetWithWall;	//���� �� ��ġ ����
	EWallRunState RunState;
	
	FVector CurrentWallPosition;
	FVector CurrentWallNormal;

	FVector LandingPoint;	//���� ����

	FVector BeforePosition2D;	//��ֹ� üũ��
};
