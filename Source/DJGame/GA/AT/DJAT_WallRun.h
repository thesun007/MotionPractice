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

	//~ UGameplayTask 재정의
	virtual void TickTask(float DeltaTime) override;	// ticking Target Actor 실행

	UFUNCTION(BlueprintCallable, Category = "Ability Tasks", Meta = (DisplayName = "TriggerTask",
	HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "True"))
		static UDJAT_WallRun* CreateWallRunTask(UGameplayAbility* OwningAbility, FName TaskInstanceName, FVector StartWallPoint, FVector StartWallNormal,
			bool Left, float _GapOffsetWithWall);

private:
	//~ UAbilityTask 재정의
	virtual void Activate() override;						
	virtual void OnDestroy(bool bInOwnerFinished) override;		//EndTask()가 실행되면 호출될 것.
	//~ UAbilityTask 끝

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
	//인터페이스를 변수로 가지는 방법.
	UPROPERTY()
	TScriptInterface<class IAnimSourceSetInterface> AnimSourceSetInterface;

	bool bLeft;		//벽이 왼쪽인지
	float MaxSpeed;
	float CurrentSpeed;
	float CurrentVirSpeed;
	float GapOffsetWithWall;	//벽과 몸 위치 차이
	EWallRunState RunState;
	
	FVector CurrentWallPosition;
	FVector CurrentWallNormal;

	FVector LandingPoint;	//착지 지점

	FVector BeforePosition2D;	//장애물 체크용
};
