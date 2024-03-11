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
	//처음 생성될 때 한번 호출
	void NativeInitializeAnimation() override;

	//virtual void NativeBeginPlay() override;
	
	//리턴이 있는 함수는 애님그래프에서 property access로 호출 가능함.
	UFUNCTION(BlueprintCallable, Category = Common, Meta = (BlueprintThreadSafe))
	class UDJAnimInstance* GetMainAnimIns() const;

};
