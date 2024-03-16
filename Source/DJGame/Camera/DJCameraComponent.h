// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "DJCameraComponent.generated.h"

/**
 * 
 */
UCLASS()
class DJGAME_API UDJCameraComponent : public UCameraComponent
{
	GENERATED_BODY()
	
public:
	UDJCameraComponent();

	//~UCameraComponent interface
	virtual void InitializeComponent() override;
	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView);	//매 프레임 호출됨(최종 카메라 transform 적용 장소)
	//~

	// Returns the camera component if one exists on the specified actor.
	UFUNCTION(BlueprintPure, Category = "Lyra|Camera")
	static UDJCameraComponent* FindCameraComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UDJCameraComponent>() : nullptr); }

private:
	//TObjectPtr<class USpringArmComponent> ParentCameraBoom;
	float blendDeltaTime;
	float CrouchOffsetblendDeltaTime;

	float AppliedCapsuleHalfHeight;

	float ApplyCapsuleHalfHeight;	//높이 변화시, 더할 보간값의 초기값
	float BlendWeightHalfHeight;
	float FinalDelta;				//최종 적용할 보간 이동 거리 (0 목표)

	FVector InitLocalLocation;		//가장 초기 카메라 로컬 위치
};
