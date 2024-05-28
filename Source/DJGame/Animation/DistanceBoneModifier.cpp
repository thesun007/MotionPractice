// Fill out your copyright notice in the Description page of Project Settings.


#include "DistanceBoneModifier.h"
#include "Animation/AnimSequence.h"
#include "AnimationBlueprintLibrary.h"
#include "EngineLogs.h"

void UDistanceBoneModifier::OnApply_Implementation(UAnimSequence* Animation)
{

	if (Animation == nullptr)
	{
		UE_LOG(LogAnimation, Error, TEXT("DistanceCurveModifier failed. Reason: Invalid Animation"));
		return;
	}

	if (!Animation->HasRootMotion())
	{
		UE_LOG(LogAnimation, Error, TEXT("DistanceCurveModifier failed. Reason: Root motion is disabled on the animation (%s)"), *GetNameSafe(Animation));
		return;
	}

	const bool bMetaDataCurve = false;
	UAnimationBlueprintLibrary::AddCurve(Animation, CurveName, ERawCurveTrackTypes::RCT_Float, bMetaDataCurve);

	const float AnimLength = Animation->GetPlayLength();
	float SampleInterval;
	int32 NumSteps;
	float TimeOfMinSpeed;

	if (bStopAtEnd)
	{
		TimeOfMinSpeed = AnimLength;
	}
	else
	{
		// Perform a high resolution search to find the sample point with minimum speed.

		TimeOfMinSpeed = 0.f;
		float MinSpeedSq = FMath::Square(StopSpeedThreshold);

		SampleInterval = 1.f / 120.f;
		NumSteps = AnimLength / SampleInterval;
		for (int32 Step = 0; Step < NumSteps; ++Step)
		{
			const float Time = Step * SampleInterval;

			const bool bAllowLooping = false;
			const FVector RootMotionTranslation = Animation->ExtractRootMotion(Time, SampleInterval, bAllowLooping).GetTranslation();
			const float RootMotionSpeedSq = CalculateMagnitudeSq(RootMotionTranslation, Axis) / SampleInterval;

			if (RootMotionSpeedSq < MinSpeedSq)
			{
				MinSpeedSq = RootMotionSpeedSq;
				TimeOfMinSpeed = Time;
			}
		}
	}

	SampleInterval = 1.f / SampleRate;
	NumSteps = FMath::CeilToInt(AnimLength / SampleInterval);
	float Time = 0.0f;

	//각 시간대별로 모두 순환
	for (int32 Step = 0; Step <= NumSteps && Time < AnimLength; ++Step)
	{
		Time = FMath::Min(Step * SampleInterval, AnimLength);

		// Assume that during any time before the stop/pivot point, the animation is approaching that point.
		// TODO: This works for clips that are broken into starts/stops/pivots, but needs to be rethought for more complex clips.
		const float ValueSign = (Time < TimeOfMinSpeed) ? -1.0f : 1.0f;

		//const FVector RootMotionTranslation = Animation->ExtractRootMotionFromRange(TimeOfMinSpeed, Time).GetTranslation();

		FVector BonePose = FVector::ZeroVector;
		int32 BoneIndex = Animation->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(BoneName);
		//목표 Bone에서 부터 root(0)까지 올라가며 부모기준으로 Transform 변환
		while (BoneIndex >= 0)
		{
			//현재 본의 로컬 트랜스폼 (부모기준 트랜스폼) (즉, 로컬 좌표를 부모기준 좌표로 변환 함)
			// 결국엔 최종 메시 스페이스기준 pose 가 계산 됨.
			FTransform CurrentTransform;
			Animation->GetBoneTransform(CurrentTransform, FSkeletonPoseBoneIndex(BoneIndex), Time, true);

			BonePose = CurrentTransform.TransformPosition(BonePose);

			if (BoneIndex == 0)
				break;

			BoneIndex = Animation->GetSkeleton()->GetReferenceSkeleton().GetParentIndex(BoneIndex);
		}

		FVector BoneTranslation = BonePose;
		UAnimationBlueprintLibrary::AddFloatCurveKey(Animation, CurveName, Time, ValueSign * CalculateMagnitude( BoneTranslation, Axis));
	}
}

void UDistanceBoneModifier::OnRevert_Implementation(UAnimSequence* Animation)
{
	const bool bRemoveNameFromSkeleton = false;
	UAnimationBlueprintLibrary::RemoveCurve(Animation, CurveName, bRemoveNameFromSkeleton);
}

float UDistanceBoneModifier::CalculateMagnitude(const FVector& Vector, EDistanceBoneCurve_Axis Axis)
{
	switch (Axis)
	{
	case EDistanceBoneCurve_Axis::X:		return Vector.X; break;
	case EDistanceBoneCurve_Axis::Y:		return Vector.Y; break;
	case EDistanceBoneCurve_Axis::Z:		return Vector.Z; break;
	default: return FMath::Sqrt(CalculateMagnitudeSq(Vector, Axis)); break;
	}
}

float UDistanceBoneModifier::CalculateMagnitudeSq(const FVector& Vector, EDistanceBoneCurve_Axis Axis)
{
	switch (Axis)
	{
		//Square : 제곱
	case EDistanceBoneCurve_Axis::X:		return FMath::Square(FMath::Abs(Vector.X)); break;
	case EDistanceBoneCurve_Axis::Y:		return FMath::Square(FMath::Abs(Vector.Y)); break;
	case EDistanceBoneCurve_Axis::Z:		return FMath::Square(FMath::Abs(Vector.Z)); break;
	case EDistanceBoneCurve_Axis::XY:		return Vector.X * Vector.X + Vector.Y * Vector.Y; break;
	case EDistanceBoneCurve_Axis::XZ:		return Vector.X * Vector.X + Vector.Z * Vector.Z; break;
	case EDistanceBoneCurve_Axis::YZ:		return Vector.Y * Vector.Y + Vector.Z * Vector.Z; break;
	case EDistanceBoneCurve_Axis::XYZ:		return Vector.X * Vector.X + Vector.Y * Vector.Y + Vector.Z * Vector.Z; break;
	default: check(false); break;
	}

	return 0.f;
}
