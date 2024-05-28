// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimationModifier.h"
#include "BlockAxisRootBoneModifier.generated.h"

UENUM(BlueprintType)
enum class EBlock_Axis : uint8
{
	X,
	Y,
	Z,
	XY,
	XZ,
	YZ,
	XYZ
};
UENUM(BlueprintType)
enum class EBlock_Type : uint8
{
	Negative,
	Positive,
	Both
};

// 특정 뼈의 전체 트랙 정보
USTRUCT(BlueprintType)
struct FBoneKeys
{
	GENERATED_BODY()
	
	FBoneKeys() {}

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FVector> Location;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FQuat> Rotation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FVector> Scale;

	void Add(const FTransform& Transform)
	{
		Location.Add(Transform.GetLocation());
		Rotation.Add(Transform.GetRotation());
		Scale.Add(Transform.GetScale3D());
	}
};
/**
 * 
 */
UCLASS()
class DJGAME_API UBlockAxisRootBoneModifier : public UAnimationModifier
{
	GENERATED_BODY()

public:

	/** Axes to calculate the distance value from. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	EBlock_Axis Axis = EBlock_Axis::Z;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	EBlock_Type BlockType = EBlock_Type::Both;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Settings)
	FBoneKeys OriginRootKeys;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Settings)
	TMap<FName, FBoneKeys> OriginChildKeys;

	UBlockAxisRootBoneModifier();

	virtual void OnApply_Implementation(UAnimSequence* Animation) override;
	virtual void OnRevert_Implementation(UAnimSequence* Animation) override;

private:
	void BlockAxis_Internal(UAnimSequence* Animation, bool isApply);

	FVector CalculateMagnitude(const FVector& OriginLocation, EBlock_Axis Axis);
};