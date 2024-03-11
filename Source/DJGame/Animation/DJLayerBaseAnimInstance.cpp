// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/DJLayerBaseAnimInstance.h"
#include "Interface/MainAnimInterface.h"

UDJLayerBaseAnimInstance::UDJLayerBaseAnimInstance()
{
}

void UDJLayerBaseAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}



UDJAnimInstance* UDJLayerBaseAnimInstance::GetMainAnimIns() const
{
	IMainAnimInterface* temp = Cast<IMainAnimInterface>(GetOwningComponent()->GetAnimInstance());
	if (temp)
		return temp->GetMainAnimIns();
	else
		return nullptr;
}
