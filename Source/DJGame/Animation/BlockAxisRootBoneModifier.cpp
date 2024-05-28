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

	//되돌리기용 데이터 지우기
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

	// 루트 뼈의 직속 자식 뼈(이름)만 추출.
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

	//** 애니메이션 데이터 편집 시작. **//
	const bool bShouldTransact = false;
	Controller.OpenBracket(LOCTEXT("ReorientingRootBone_Bracket", "Reorienting root bone"), bShouldTransact);

	FBoneKeys NewRootKeys;					// 루트 뼈의 전체 트랙 정보.
	TMap<FName, FBoneKeys> NewChildKeys;	// 직속 자식 뼈들의 전체 트랙 정보.

	// 애니메이션 전체 트랙 루프
	const int32 Num = Model->GetNumberOfKeys();
	for (int32 AnimKey = 0; AnimKey < Num; AnimKey++)
	{
		// 현재 트랙 키(AnimKey) 에서의 루트 뼈 트랜스폼 얻기.
		const FTransform RootTransformOriginal = Model->EvaluateBoneTrackTransform(RootBoneName, AnimKey, EAnimInterpolationType::Step);

		// 현재 트랙 키(AnimKey) 에서의 새로운 루트 뼈 트랜스폼 계산
		FTransform RootTransformNew = RootTransformOriginal;
		if (isApply)
		{
			OriginRootKeys.Add(RootTransformOriginal);

			RootTransformNew.SetLocation(CalculateMagnitude(RootTransformNew.GetLocation(), Axis));
			NewRootKeys.Add(RootTransformNew);

			// 이제 자식 뼈 트랜스폼 처리.
			for (FName ChildBoneName : RootBoneChildBones)
			{
				FTransform ChildransformOriginal = Model->EvaluateBoneTrackTransform(ChildBoneName, AnimKey, EAnimInterpolationType::Step);

				// 되돌리기를 위한 기존 정보 따로 저장
				OriginChildKeys.FindOrAdd(ChildBoneName).Add(ChildransformOriginal);
				
				// 골반 아니라면 스킵.(속 빈 뼈 아이콘)
				if (ChildBoneName.Compare(FName("pelvis")) != 0)
					continue;

				// 자식 뼈 로컬 트랜스폼을 메시 컴포넌트 기준(such as local to world) 으로 바꿧다가
				ChildransformOriginal = ChildransformOriginal * RootTransformOriginal;	//크자이공부
				// 새 루트 뼈 기준 로컬 트랜스폼으로 재 변환.
				ChildransformOriginal = ChildransformOriginal.GetRelativeTransform(RootTransformNew);

				// 현재 트랙 키에서 자식의 새로운 로컬 트랜스폼을 저장
				FBoneKeys& ChildKeys = NewChildKeys.FindOrAdd(ChildBoneName);
				ChildKeys.Add(ChildransformOriginal);
			}
		}
		else
		{
			// 되돌리기라면 저장 데이터로 채우기.
			FTransform temp;
			temp.SetLocation(OriginRootKeys.Location[AnimKey]);
			temp.SetRotation(OriginRootKeys.Rotation[AnimKey]);
			temp.SetScale3D(OriginRootKeys.Scale[AnimKey]);
			NewRootKeys.Add(temp);

			for (FName ChildBoneName : RootBoneChildBones)
			{
				// 골반 아니라면 스킵.(속 빈 뼈 아이콘)
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

	// 루트 뼈의 새 전체 트랙 정보를 애니메이션에 적용
	Controller.SetBoneTrackKeys(RootBoneName, NewRootKeys.Location, NewRootKeys.Rotation, NewRootKeys.Scale);

	// 자식 뼈의 새 전체 트랙 정보를 애니메이션에 적용
	for (const FName& ChildBoneName : RootBoneChildBones)
	{
		// 골반 아니라면 스킵.(속 빈 뼈 아이콘)
		if (ChildBoneName.Compare(FName("pelvis")) != 0)
			continue;

		const FBoneKeys& ChildKeys = NewChildKeys.FindChecked(ChildBoneName);
		Controller.SetBoneTrackKeys(ChildBoneName, ChildKeys.Location, ChildKeys.Rotation, ChildKeys.Scale);
	}

	//** 애니메이션 데이터 편집 끝. **//
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