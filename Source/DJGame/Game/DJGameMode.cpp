// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/DJGameMode.h"
#include "Player/DJPlayerController.h"


ADJGameMode::ADJGameMode()
{
	/*static ConstructorHelpers::FClassFinder<APawn> DefaultPawnClassRef(TEXT("/Game/DJGame/Blueprints/BP_DJCharacterPlayer.BP_DJCharacterPlayer_C"));
	if (DefaultPawnClassRef.Class)
	{
		DefaultPawnClass = DefaultPawnClassRef.Class;
	}*/
	
	static ConstructorHelpers::FClassFinder<APlayerController> DefaultPlayerControllerClassRef(TEXT("/Game/DJGame/Blueprints/BP_PlayerController.BP_PlayerController_C"));
	if (DefaultPlayerControllerClassRef.Class)
	{
		PlayerControllerClass = DefaultPlayerControllerClassRef.Class;
	}

}
