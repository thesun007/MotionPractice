// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/BlockAxisRootBoneModifier.h"
//#include "EditorDialogLibrary.h"
#include "Misc/MessageDialog.h"
#include "Animation/AnimData/AnimDataModel.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimData/IAnimationDataController.h"
//#include "AnimationUtils.h"
#include "Animation/Skeleton.h"
#include "EngineLogs.h"

#define LOCTEXT_NAMESPACE "BlockAxisRootBoneModifier"

UBlockAxisRootBoneModifier::UBlockAxisRootBoneModifier()
{
}

void UBlockAxisRootBoneModifier::OnApply_Implementation(UAnimSequence* Animation)
{
	BlockAxis_Internal(Animation, true);
}

void UBlockAxisRootBoneModifier::OnRevert_Implementation(UAnimSequence* Animation)
{
	BlockAxis_Internal(Animation, false);

	//�ǵ������ ������ �����
	OriginRootKeys = FBoneKeys();
	OriginChildKeys.Reset();
}

void UBlockAxisRootBoneModifier::BlockAxis_Internal(UAnimSequence* Animation, bool isApply)
{
	if (isApply)
	{
		EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::YesNo,
			FText::FromString(TEXT("Are you sure you want to proceed? \n(If the save is deleted, Revert will not work..)")));
		if (Result == EAppReturnType::No) {
			return;
		}
	}
	else
	{
		if(OriginRootKeys.Location.IsEmpty() || OriginChildKeys.IsEmpty())
			return;
	}

	IAnimationDataController& Controller = Animation->GetController();
	const IAnimationDataModel* Model = Animation->GetDataModel();

	if (Model == nullptr)
	{
		UE_LOG(LogAnimation, Error, TEXT("ReOrientMeshModifier failed. Reason: Invalid Data Model. Animation: %s"), *GetNameSafe(Animation));
		return;
	}

	const USkeleton* Skeleton = Animation->GetSkeleton();
	if (Skeleton == nullptr)
	{
		UE_LOG(LogAnimation, Error, TEXT("ReOrientMeshModifier failed. Reason: Invalid Skeleton. Animation: %s"), *GetNameSafe(Animation));
		return;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	if (RefSkeleton.GetNum() == 0)
	{
		UE_LOG(LogAnimation, Error, TEXT("ReOrientMeshModifier failed. Reason: Ref Skeleton. Animation: %s"), *GetNameSafe(Animation));
		return;
	}

	// ��Ʈ ���� ���� �ڽ� ��(�̸�)�� ����.
	const FName RootBoneName = RefSkeleton.GetBoneName(0);
	TArray<FName> RootBoneChildBones;

	TArray<int32> ChildBoneIndices;
	RefSkeleton.GetDirectChildBones(0, ChildBoneIndices);

	for (const int32& ChildBoneIndex : ChildBoneIndices)
	{
		const FName ChildBoneName = RefSkeleton.GetBoneName(ChildBoneIndex);
		if (Model->IsValidBoneTrackName(ChildBoneName))
		{
			RootBoneChildBones.Add(ChildBoneName);
		}
	}

	//** �ִϸ��̼� ������ ���� ����. **//
	const bool bShouldTransact = false;
	Controller.OpenBracket(LOCTEXT("ReorientingRootBone_Bracket", "Reorienting root bone"), bShouldTransact);

	FBoneKeys NewRootKeys;					// ��Ʈ ���� ��ü Ʈ�� ����.
	TMap<FName, FBoneKeys> NewChildKeys;	// ���� �ڽ� ������ ��ü Ʈ�� ����.

	// �ִϸ��̼� ��ü Ʈ�� ����
	const int32 Num = Model->GetNumberOfKeys();
	for (int32 AnimKey = 0; AnimKey < Num; AnimKey++)
	{
		// ���� Ʈ�� Ű(AnimKey) ������ ��Ʈ �� Ʈ������ ���.
		const FTransform RootTransformOriginal = Model->EvaluateBoneTrackTransform(RootBoneName, AnimKey, EAnimInterpolationType::Step);

		// ���� Ʈ�� Ű(AnimKey) ������ ���ο� ��Ʈ �� Ʈ������ ���
		FTransform RootTransformNew = RootTransformOriginal;
		if (isApply)
		{
			OriginRootKeys.Add(RootTransformOriginal);

			RootTransformNew.SetLocation(CalculateMagnitude(RootTransformNew.GetLocation(), Axis));
			NewRootKeys.Add(RootTransformNew);

			// ���� �ڽ� �� Ʈ������ ó��.
			for (FName ChildBoneName : RootBoneChildBones)
			{
				FTransform ChildransformOriginal = Model->EvaluateBoneTrackTransform(ChildBoneName, AnimKey, EAnimInterpolationType::Step);

				// �ǵ����⸦ ���� ���� ���� ���� ����
				OriginChildKeys.FindOrAdd(ChildBoneName).Add(ChildransformOriginal);
				
				// ��� �ƴ϶�� ��ŵ.(�� �� �� ������)
				if (ChildBoneName.Compare(FName("pelvis")) != 0)
					continue;

				// �ڽ� �� ���� Ʈ�������� �޽� ������Ʈ ����(such as local to world) ���� �مf�ٰ�
				ChildransformOriginal = ChildransformOriginal * RootTransformOriginal;	//ũ���̰���
				// �� ��Ʈ �� ���� ���� Ʈ���������� �� ��ȯ.
				ChildransformOriginal = ChildransformOriginal.GetRelativeTransform(RootTransformNew);

				// ���� Ʈ�� Ű���� �ڽ��� ���ο� ���� Ʈ�������� ����
				FBoneKeys& ChildKeys = NewChildKeys.FindOrAdd(ChildBoneName);
				ChildKeys.Add(ChildransformOriginal);
			}
		}
		else
		{
			// �ǵ������� ���� �����ͷ� ä���.
			FTransform temp;
			temp.SetLocation(OriginRootKeys.Location[AnimKey]);
			temp.SetRotation(OriginRootKeys.Rotation[AnimKey]);
			temp.SetScale3D(OriginRootKeys.Scale[AnimKey]);
			NewRootKeys.Add(temp);

			for (FName ChildBoneName : RootBoneChildBones)
			{
				// ��� �ƴ϶�� ��ŵ.(�� �� �� ������)
				if (ChildBoneName.Compare(FName("pelvis")) != 0)
					continue;

				temp.SetLocation(OriginChildKeys[ChildBoneName].Location[AnimKey]);
				temp.SetRotation(OriginChildKeys[ChildBoneName].Rotation[AnimKey]);
				temp.SetScale3D(OriginChildKeys[ChildBoneName].Scale[AnimKey]);

				FBoneKeys& ChildKeys = NewChildKeys.FindOrAdd(ChildBoneName);
				ChildKeys.Add(temp);
			}

		}

	}

	// ��Ʈ ���� �� ��ü Ʈ�� ������ �ִϸ��̼ǿ� ����
	Controller.SetBoneTrackKeys(RootBoneName, NewRootKeys.Location, NewRootKeys.Rotation, NewRootKeys.Scale);

	// �ڽ� ���� �� ��ü Ʈ�� ������ �ִϸ��̼ǿ� ����
	for (const FName& ChildBoneName : RootBoneChildBones)
	{
		// ��� �ƴ϶�� ��ŵ.(�� �� �� ������)
		if (ChildBoneName.Compare(FName("pelvis")) != 0)
			continue;

		const FBoneKeys& ChildKeys = NewChildKeys.FindChecked(ChildBoneName);
		Controller.SetBoneTrackKeys(ChildBoneName, ChildKeys.Location, ChildKeys.Rotation, ChildKeys.Scale);
	}

	//** �ִϸ��̼� ������ ���� ��. **//
	Controller.CloseBracket(bShouldTransact);

	Animation->PostEditChange();
}

FVector UBlockAxisRootBoneModifier::CalculateMagnitude(const FVector& OriginLocation, EBlock_Axis _Axis)
{
	FVector TargetLocation(.0, .0, .0);
	if (BlockType == EBlock_Type::Negative)
	{
		TargetLocation.X = OriginLocation.X < 0 ? 0 : OriginLocation.X;
		TargetLocation.Y = OriginLocation.Y < 0 ? 0 : OriginLocation.Y;
		TargetLocation.Z = OriginLocation.Z < 0 ? 0 : OriginLocation.Z;
	}
	else if (BlockType == EBlock_Type::Positive)
	{
		TargetLocation.X = OriginLocation.X > 0 ? 0 : OriginLocation.X;
		TargetLocation.Y = OriginLocation.Y > 0 ? 0 : OriginLocation.Y;
		TargetLocation.Z = OriginLocation.Z > 0 ? 0 : OriginLocation.Z;
	}

	switch (_Axis)
	{
	case EBlock_Axis::X:
		return FVector(TargetLocation.X, OriginLocation.Y, OriginLocation.Z); break;
	case EBlock_Axis::Y:	
		return FVector(OriginLocation.X, TargetLocation.Y, OriginLocation.Z); break;
	case EBlock_Axis::Z:	
		return FVector(OriginLocation.X, OriginLocation.Y, TargetLocation.Z); break;
	case EBlock_Axis::XY:	
		return FVector(TargetLocation.X, TargetLocation.Y, OriginLocation.Z); break;
	case EBlock_Axis::XZ:	
		return FVector(TargetLocation.X, OriginLocation.Y, TargetLocation.Z); break;
	case EBlock_Axis::YZ:	
		return FVector(OriginLocation.X, TargetLocation.Y, TargetLocation.Z); break;
	case EBlock_Axis::XYZ:	
		return FVector(TargetLocation.X, TargetLocation.Y, TargetLocation.Z); break;
	default: check(false); break;
	}

	return FVector();
}

#undef LOCTEXT_NAMESPACE