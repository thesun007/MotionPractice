// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AnimSourceSetInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UAnimSourceSetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DJGAME_API IAnimSourceSetInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void SetDirectionWall(uint8 bLeft) = 0;
	virtual void SetDisplacementSpeedWithWall(float speed) = 0;
	virtual void SetGroundDistanceByWallRun(float distance) = 0;
};
