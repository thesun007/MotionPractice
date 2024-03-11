// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DJLayerBaseAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class DJGAME_API UDJLayerBaseAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UDJLayerBaseAnimInstance();

protected:
	//ó�� ������ �� �ѹ� ȣ��
	void NativeInitializeAnimation() override;

	//virtual void NativeBeginPlay() override;
	
	//������ �ִ� �Լ��� �ִԱ׷������� property access�� ȣ�� ������.
	UFUNCTION(BlueprintCallable, Category = Common, Meta = (BlueprintThreadSafe))
	class UDJAnimInstance* GetMainAnimIns() const;

};
