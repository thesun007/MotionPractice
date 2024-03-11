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
	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView);
	//~

	// Returns the camera component if one exists on the specified actor.
	UFUNCTION(BlueprintPure, Category = "Lyra|Camera")
	static UDJCameraComponent* FindCameraComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UDJCameraComponent>() : nullptr); }

private:
	//TObjectPtr<class USpringArmComponent> ParentCameraBoom;
	float blendDeltaTime;
	float CrouchOffsetblendDeltaTime;

	float AppliedCapsuleHalfHeight;

	float ApplyCapsuleHalfHeight;
	float BlendWeightHalfHeight;
	float FinalDelta;	//보간 이동 거리 (0 목표)

	FVector InitLocalLocation;
};
