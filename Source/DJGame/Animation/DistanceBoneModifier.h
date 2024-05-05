// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimationModifier.h"
#include "DistanceBoneModifier.generated.h"


// @todo: Consolidate with EMotionExtractor_Axis
/** Axes to calculate the distance value from */
UENUM(BlueprintType)
enum class EDistanceBoneCurve_Axis : uint8
{
	X,
	Y,
	Z,
	XY,
	XZ,
	YZ,
	XYZ
};

/**
 * 
 */
UCLASS()
class DJGAME_API UDistanceBoneModifier : public UAnimationModifier
{
	GENERATED_BODY()
	
public:

	/** Rate used to sample the animation. */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (ClampMin = "1"))
	int32 SampleRate = 30;

	/** Name for the generated curve. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	FName CurveName = "DistanceBone";

	/** Root motion speed must be below this threshold to be considered stopped. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (EditCondition = "!bStopAtEnd"))
	float StopSpeedThreshold = 5.0f;

	/** Axes to calculate the distance value from. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	EDistanceBoneCurve_Axis Axis = EDistanceBoneCurve_Axis::XY;

	/** Root motion is considered to be stopped at the clip's end */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	bool bStopAtEnd = false;

	/** 추출할 Bone 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	FName BoneName;

	virtual void OnApply_Implementation(UAnimSequence* Animation) override;
	virtual void OnRevert_Implementation(UAnimSequence* Animation) override;

private:

	/** Helper functions to calculate the magnitude of a vector only considering a specific axis or axes */
	static float CalculateMagnitude(const FVector& Vector, EDistanceBoneCurve_Axis Axis);
	static float CalculateMagnitudeSq(const FVector& Vector, EDistanceBoneCurve_Axis Axis);
};
